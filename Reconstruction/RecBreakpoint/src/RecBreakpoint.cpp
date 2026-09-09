#include "RecBreakpoint.h"
#include "BreakpointFitter.h"
#include "BreakpointTrackSystem.h"
#include "TruthBHLossEventData.h"
#include "GearSvc/IGearSvc.h"
#include "TrackSystemSvc/ITrackSystemSvc.h"
#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "TFile.h"
#include "TTree.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <set>
#include <iomanip>
#include <sstream>
#include <numeric>

DECLARE_COMPONENT(RecBreakpoint)

RecBreakpoint::RecBreakpoint(const std::string& name, ISvcLocator* locator)
    : Algorithm(name, locator) {
  declareProperty("InputTracks", m_input, "Tracks whose reconstructed hits are refitted");
  declareProperty("OutputTracks", m_output, "Successful refitted tracks");
  declareProperty("OutputTracksBackwardFilter", m_backwardOutput, "Parallel backward-filter tracks");
}
RecBreakpoint::~RecBreakpoint() = default;

StatusCode RecBreakpoint::initialize() {
  if (Algorithm::initialize().isFailure()) return StatusCode::FAILURE;
  try {
    breakpoint::parseSeedHitSelection(m_seedHitSelection.value());
  } catch (const std::exception& exception) {
    error() << exception.what() << endmsg;
    return StatusCode::FAILURE;
  }
  m_seedSelectionName = m_seedHitSelection.value();
  m_lossStateModeName = m_lossStateMode.value();
  const bool canIterate = m_lossStateModeName == "Persistent6D" || m_lossStateModeName == "LocalMarginal";
  if (m_maxIterations<1 || m_maxIterations>20 || !std::isfinite(m_iterationTolerance) ||
      m_iterationTolerance<=0 || (m_maxIterations>1 &&
      (!canIterate || m_intervals.value().size()!=1))) {
    error() << "MaxFitIterations must be 1..20, tolerance positive; iterations require"
            << " Persistent6D or LocalMarginal, and exactly one breakpoint" << endmsg;
    return StatusCode::FAILURE;
  }
  if ((m_lossStateModeName != "Persistent6D" && m_lossStateModeName != "LocalMarginal"
       && m_lossStateModeName != "TruthOverride") ||
      (m_lossStateModeName == "Persistent6D" && m_intervals.value().size() > 1)) {
    error() << "LossStateMode must be LocalMarginal, Persistent6D or TruthOverride; Persistent6D requires"
            << " at most one breakpoint" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_output.fullKey() == m_backwardOutput.fullKey()) {
    error() << "The two output track collections must have different names" << endmsg;
    return StatusCode::FAILURE;
  }
  std::set<int> unique;
  for (int interval : m_intervals.value()) {
    if (interval < 0 || !unique.insert(interval).second) {
      error() << "BreakpointIntervals requires distinct nonnegative indices" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  for (double value : {m_seedScale.value(), m_backwardSeedScale.value(),
                       m_maxChi2.value(), m_truthEndpointDistance.value()})
    if (!std::isfinite(value) || value <= 0) {
      error() << "SeedScale, BackwardSeedScale, MaxChi2PerHit, TruthMaxEndpointDistance"
              << " must be finite and positive" << endmsg;
      return StatusCode::FAILURE;
    }
  if (m_lossStateModeName != "TruthOverride" &&
      (!std::isfinite(m_meanLoss.value()) || m_meanLoss < 0 || m_meanLoss > 5 ||
       !std::isfinite(m_sigmaLoss.value()) || m_sigmaLoss <= 0)) {
    error() << "MeanLogLoss must be finite in [0,5] and SigmaLogLoss finite/positive" << endmsg;
    return StatusCode::FAILURE;
  }
  const auto geometry = service<IGeomSvc>("GeomSvc");
  const auto gear = service<IGearSvc>("GearSvc");
  if (!geometry || !gear || !gear->getGearMgr()) return StatusCode::FAILURE;
  const auto field = geometry->lcdd()->field().magneticField(dd4hep::Position(0, 0, 0));
  m_bz = field.z() / dd4hep::tesla;
  if (!std::isfinite(m_bz) || m_bz == 0) return StatusCode::FAILURE;
  // Match the ready CompleteTracks/GSF measurement backend. The separate
  // DDKalTest backend has different layer lookup behavior in this geometry.
  m_system = std::make_unique<breakpoint::BreakpointTrackSystem>(*gear->getGearMgr());
  if (!m_system) return StatusCode::FAILURE;
  m_system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useQMS, m_ms);
  m_system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::usedEdx, m_eloss);
  m_system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing, false);
  m_system->init();

  const std::filesystem::path path(m_tupleName.value());
  if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
  // Refuse accidental overwrites of a previous diagnostic.
  m_file.reset(TFile::Open(path.c_str(), "CREATE"));
  if (!m_file || m_file->IsZombie()) return StatusCode::FAILURE;
  m_file->cd();
  m_tree = new TTree("breakpoint", "Selected-interval breakpoint KF diagnostics");
  m_recordBackwardSeedScale = m_backwardSeedScale.value();
  m_tree->Branch("backward_seed_scale", &m_recordBackwardSeedScale);
  m_tree->Branch("event_index", &m_event);
  m_tree->Branch("input_track_index", &m_trackIndex);
  m_tree->Branch("status", &m_fitStatus);
  m_tree->Branch("hit_count", &m_hitCount);
  m_tree->Branch("truth_pt", &m_truthPt);
  m_tree->Branch("kf_pt", &m_kfPt);
  m_tree->Branch("breakpoint_pt", &m_fitPt);
  m_tree->Branch("filter_chi2", &m_fitChi2);
  m_tree->Branch("reference_kf_pt", &m_referencePt);
  m_tree->Branch("seed_hit_selection", &m_seedSelectionName);
  m_tree->Branch("seed_hit_indices", &m_seedHitIndices);
  m_tree->Branch("rts_pt", &m_fitPt);
  m_tree->Branch("backward_pt", &m_backwardPt);
  m_tree->Branch("forward_chi2", &m_fitChi2);
  m_tree->Branch("backward_chi2", &m_backwardTotalChi2);
  m_tree->Branch("smoothed_chi2", &m_smoothedTotalChi2);
  m_tree->Branch("smoothed_chi2_status", &m_smoothedChi2Status);
  m_tree->Branch("smoothed_chi2_error", &m_smoothedChi2Error);
  m_tree->Branch("forward_local_chi2", &m_localChi2);
  m_tree->Branch("smoothed_local_chi2", &m_smoothedChi2);
  m_tree->Branch("smoothed_measurement_chi2", &m_smoothedMeasurementChi2);
  m_tree->Branch("smoothed_process_chi2", &m_smoothedProcessChi2);
  m_tree->Branch("smoothed_native_measurement_chi2", &m_smoothedNativeChi2);
  m_tree->Branch("smoothed_seed_chi2", &m_smoothedSeedChi2);
  m_tree->Branch("backward_seed_forward_chi2", &m_backwardSeedForwardChi2);
  m_tree->Branch("backward_fitted_log_loss", &m_backwardLoss);
  m_tree->Branch("backward_fitted_log_loss_variance", &m_backwardLossVariance);
  m_tree->Branch("reference_backward_kf_pt", &m_backwardReferencePt);
  m_tree->Branch("backward_fit_iterations", &m_backwardIterations);
  m_tree->Branch("backward_iteration_status", &m_backwardIterationStatus);
  m_tree->Branch("backward_iteration_error", &m_backwardIterationError);
  m_tree->Branch("backward_iteration_pt", &m_backwardIterationPt);
  m_tree->Branch("backward_iteration_log_loss", &m_backwardIterationLoss);
  m_tree->Branch("backward_iteration_log_loss_variance", &m_backwardIterationVariance);
  m_tree->Branch("backward_iteration_step_norm", &m_backwardIterationNorm);
  m_tree->Branch("backward_iteration_linearized_chi2", &m_backwardIterationChi2);
  m_tree->Branch("loss_state_mode", &m_lossStateModeName);
  m_tree->Branch("truth_override_status", &m_truthOverrideStatus);
  m_tree->Branch("truth_override_error", &m_truthOverrideError);
  m_tree->Branch("truth_override_g4_track_id", &m_truthG4Track);
  m_tree->Branch("truth_override_max_endpoint_distance", &m_truthMaxDistance);
  m_tree->Branch("truth_override_interval", &m_truthIntervals);
  m_tree->Branch("truth_override_retained_fraction", &m_truthZ);
  m_tree->Branch("truth_override_log_loss", &m_truthB);
  m_tree->Branch("truth_override_momentum_before", &m_truthMomentumBefore);
  m_tree->Branch("truth_override_ebrem_loss", &m_truthEbremLoss);
  m_tree->Branch("truth_override_tx0", &m_truthTX0);
  m_tree->Branch("truth_override_first_step", &m_truthFirstStep);
  m_tree->Branch("truth_override_last_step", &m_truthLastStep);
  m_tree->Branch("truth_override_start_fraction", &m_truthStartFraction);
  m_tree->Branch("truth_override_end_fraction", &m_truthEndFraction);
  m_tree->Branch("one_pass_pt",&m_onePassPt);
  m_tree->Branch("fit_iterations",&m_iterations);
  m_tree->Branch("iteration_status",&m_iterationStatus);
  m_tree->Branch("iteration_error",&m_iterationError);
  m_tree->Branch("iteration_pt",&m_iterationPt);
  m_tree->Branch("iteration_log_loss",&m_iterationLoss);
  m_tree->Branch("iteration_log_loss_variance",&m_iterationLossVariance);
  m_tree->Branch("iteration_step_norm",&m_iterationNorm);
  m_tree->Branch("iteration_linearized_chi2",&m_iterationChi2);
  m_tree->Branch("persistent_hit_index", &m_persistentHits);
  m_tree->Branch("persistent_predicted_mean", &m_sixPredictedMean);
  m_tree->Branch("persistent_predicted_covariance", &m_sixPredictedCov);
  m_tree->Branch("persistent_filtered_mean", &m_sixFilteredMean);
  m_tree->Branch("persistent_filtered_covariance", &m_sixFilteredCov);
  m_tree->Branch("persistent_smoothed_mean", &m_sixSmoothedMean);
  m_tree->Branch("persistent_smoothed_covariance", &m_sixSmoothedCov);
  m_tree->Branch("persistent_transport", &m_sixTransport);
  m_tree->Branch("persistent_process_noise", &m_sixNoise);
  m_tree->Branch("hit_cell_id", &m_hitCell);
  m_tree->Branch("hit_r_mm", &m_hitR);
  m_tree->Branch("hit_z_mm", &m_hitZ);
  m_tree->Branch("local_chi2", &m_localChi2);
  m_tree->Branch("filtered_kappa", &m_filteredKappa);
  m_tree->Branch("smoothed_kappa", &m_smoothedKappa);
  m_tree->Branch("filtered_kappa_variance", &m_filteredVariance);
  m_tree->Branch("smoothed_kappa_variance", &m_smoothedVariance);
  m_tree->Branch("backward_predicted_kappa", &m_backwardPredictedKappa);
  m_tree->Branch("backward_predicted_kappa_variance", &m_backwardPredictedVariance);
  m_tree->Branch("backward_filtered_kappa", &m_backwardKappa);
  m_tree->Branch("backward_filtered_kappa_variance", &m_backwardVariance);
  m_tree->Branch("backward_local_chi2", &m_backwardChi2);
  m_tree->Branch("breakpoint_interval", &m_breakpointIndex);
  m_tree->Branch("prior_log_loss", &m_priorLoss);
  m_tree->Branch("local_log_loss", &m_localLoss);
  m_tree->Branch("local_log_loss_variance", &m_localVariance);
  m_tree->Branch("fitted_log_loss", &m_fittedLoss);
  m_tree->Branch("fitted_log_loss_variance", &m_lossVariance);
  m_tree->Branch("smoothed_log_loss", &m_rtsLoss);
  m_tree->Branch("smoothed_log_loss_variance", &m_rtsLossVariance);
  m_tree->Branch("covariance_transport_closure", &m_closure);
  info() << "Outward KF + parallel RTS and BackwardFilter; " << m_intervals.value().size()
         << " selected breakpoint intervals; seed=" << m_seedSelectionName
         << "; Bz=" << m_bz << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecBreakpoint::execute() {
  ++m_event;
  auto* output = m_output.createAndPut();
  auto* backwardOutput = m_backwardOutput.createAndPut();
  auto* statuses = m_status.createAndPut();
  auto* outputIndices = m_outputIndex.createAndPut();
  auto* backwardOutputIndices = m_backwardOutputIndex.createAndPut();
  const auto* tracks = m_input.get();
  const bool selected = m_selected.value().empty() ||
      std::find(m_selected.value().begin(), m_selected.value().end(), m_event) != m_selected.value().end();
  const double nan = std::numeric_limits<double>::quiet_NaN();
  m_truthPt = nan;
  if (selected && m_truthDiagnostics) {
    // Optional reference only, never used by the fitter. Ambiguous multi-electron
    // events have no scalar truth reference; this is not track truth matching.
    int count = 0;
    for (const auto& particle : *m_truth.get())
      if (particle.getGeneratorStatus() == 1 && std::abs(particle.getPDG()) == 11) {
        const auto p = particle.getMomentum();
        m_truthPt = std::hypot(p.x, p.y);
        ++count;
      }
    if (count != 1) m_truthPt = nan;
  }
  breakpoint::KalmanAdapter adapter(m_system.get(), m_bz, m_maxChi2,
      breakpoint::parseSeedHitSelection(m_seedHitSelection.value()));
  breakpoint::BreakpointFitter fitter(adapter);
  breakpoint::FitSettings settings;
  settings.intervals = m_intervals.value();
  settings.meanLogLoss = m_meanLoss;
  settings.sigmaLogLoss = m_sigmaLoss;
  settings.seedScale = m_seedScale;
  settings.backwardSeedScale = m_backwardSeedScale;
  settings.lossStateMode = m_lossStateModeName;
  settings.maxFitIterations=m_maxIterations;
  settings.relinearizationTolerance=m_iterationTolerance;
  const bool needTruthLoss = selected && m_lossStateModeName == "TruthOverride"
      && !settings.intervals.empty();
  TruthBHLossEventData truthReader; // event-local maps, released after this event
  bool truthPrepared = false;
  std::string truthEventError;
  if (needTruthLoss) {
    try {
      std::vector<const edm4hep::MCRecoTrackerAssociationCollection*> associations;
      auto addAvailable = [&associations](auto& handle) {
        try { if (const auto* collection = handle.get()) associations.push_back(collection); }
        catch (...) {} // An unused detector may be absent; actual track matches stay strict.
      };
      addAvailable(m_vxdAssociations); addAvailable(m_itkbAssociations);
      addAvailable(m_itkeAssociations); addAvailable(m_tpcAssociations);
      addAvailable(m_otkbAssociations); addAvailable(m_otkeAssociations);
      truthPrepared = truthReader.prepare(m_truthSteps.get(), m_truthLinks.get(),
                                         associations, truthEventError);
    } catch (const std::exception& error) { truthEventError = error.what(); }
    catch (...) { truthEventError = "Cannot retrieve embedded truth collections"; }
  }
  m_trackIndex = -1;
  for (const auto& track : *tracks) {
    ++m_trackIndex;
    if (!selected) { statuses->push_back(0); outputIndices->push_back(-1); backwardOutputIndices->push_back(-1); continue; }
    m_fitStatus = -1;
    settings.truthLogLoss.clear();
    m_truthOverrideStatus = 0; m_truthG4Track = -1; m_truthMaxDistance = nan;
    m_truthOverrideError.clear(); m_truthIntervals.clear(); m_truthZ.clear(); m_truthB.clear();
    m_truthMomentumBefore.clear(); m_truthEbremLoss.clear(); m_truthTX0.clear();
    m_truthFirstStep.clear(); m_truthLastStep.clear();
    m_truthStartFraction.clear(); m_truthEndFraction.clear();
    m_iterations=0; m_iterationStatus=0; m_onePassPt=nan; m_iterationError.clear();
    m_iterationPt.clear();m_iterationLoss.clear();m_iterationLossVariance.clear();
    m_iterationNorm.clear();m_iterationChi2.clear();
    m_seedHitIndices.clear();
    m_persistentHits.clear();
    m_sixPredictedMean.clear(); m_sixPredictedCov.clear();
    m_sixFilteredMean.clear(); m_sixFilteredCov.clear();
    m_sixSmoothedMean.clear(); m_sixSmoothedCov.clear();
    m_sixTransport.clear(); m_sixNoise.clear();
    m_kfPt = m_fitPt = m_fitChi2 = m_referencePt = nan;
    m_backwardPt=m_backwardTotalChi2=m_smoothedTotalChi2=m_smoothedSeedChi2=nan;
    m_backwardSeedForwardChi2=m_backwardReferencePt=nan;
    m_smoothedChi2Status=m_backwardIterations=m_backwardIterationStatus=0;
    m_smoothedChi2Error.clear();m_backwardIterationError.clear();
    m_smoothedChi2.clear();m_smoothedMeasurementChi2.clear();m_smoothedProcessChi2.clear();m_smoothedNativeChi2.clear();
    m_backwardLoss.clear();m_backwardLossVariance.clear();
    m_backwardIterationPt.clear();m_backwardIterationLoss.clear();m_backwardIterationVariance.clear();
    m_backwardIterationNorm.clear();m_backwardIterationChi2.clear();
    m_hitCount = 0;
    m_hitCell.clear(); m_hitR.clear(); m_hitZ.clear(); m_localChi2.clear();
    m_filteredKappa.clear(); m_smoothedKappa.clear();
    m_filteredVariance.clear(); m_smoothedVariance.clear();
    m_backwardKappa.clear(); m_backwardVariance.clear(); m_backwardChi2.clear();
    m_backwardPredictedKappa.clear(); m_backwardPredictedVariance.clear();
    m_rtsLoss.clear(); m_rtsLossVariance.clear();
    m_breakpointIndex.clear(); m_priorLoss.clear(); m_localLoss.clear();
    m_localVariance.clear(); m_fittedLoss.clear(); m_lossVariance.clear(); m_closure.clear();
    for (const auto& state : track.getTrackStates())
      if (state.location == 1 && state.omega != 0)
        m_kfPt = std::abs(m_bz * 2.99792458e-4 / state.omega);
    try {
      std::vector<edm4hep::TrackerHit> hits;
      for (auto hit : track.getTrackerHits()) hits.push_back(hit);
      // First working version: outward barrel ordering, with explicit indices
      // and positions saved so an interval cannot be confused with a layer ID.
      std::stable_sort(hits.begin(), hits.end(), [](auto a, auto b) {
        auto pa = a.getPosition(), pb = b.getPosition();
        return pa.x * pa.x + pa.y * pa.y < pb.x * pb.x + pb.y * pb.y;
      });
      m_hitCount = hits.size();
      for (auto hit : hits) {
        auto p = hit.getPosition();
        m_hitCell.push_back(hit.getCellID());
        m_hitR.push_back(std::hypot(p.x, p.y));
        m_hitZ.push_back(p.z);
      }
      if (needTruthLoss) {
        if (!truthPrepared) {
          m_truthOverrideStatus = -1;
          m_truthOverrideError = truthEventError;
          throw std::runtime_error("TruthOverride event data: " + truthEventError);
        }
        TruthBHLossEventDataMatch match;
        const bool matched = truthReader.matchTrack(hits, m_truthEndpointDistance, true,
                                                    match, m_truthOverrideError);
        m_truthG4Track = match.g4TrackID;
        m_truthMaxDistance = match.maxEndpointDistance;
        if (!matched) {
          m_truthOverrideStatus = -2;
          throw std::runtime_error("TruthOverride track association: " + m_truthOverrideError);
        }
        for (int interval : settings.intervals) {
          if (interval < 0 || interval >= static_cast<int>(match.materialIntervals.size())) {
            m_truthOverrideStatus = -3;
            m_truthOverrideError = "Configured interval is outside the matched track";
            throw std::runtime_error(m_truthOverrideError);
          }
        }
        for (int interval : settings.intervals) {
          const auto& truth = match.materialIntervals[interval];
          const double b = -std::log(truth.retainedFraction);
          settings.truthLogLoss.emplace(interval, b);
          m_truthIntervals.push_back(interval); m_truthZ.push_back(truth.retainedFraction);
          m_truthB.push_back(b); m_truthMomentumBefore.push_back(truth.momentumBefore);
          m_truthEbremLoss.push_back(truth.ebremLoss); m_truthTX0.push_back(truth.truthTX0);
          m_truthFirstStep.push_back(truth.firstStepNumber); m_truthLastStep.push_back(truth.lastStepNumber);
          m_truthStartFraction.push_back(truth.startHookFraction); m_truthEndFraction.push_back(truth.endHookFraction);
          if (m_verbose) info() << std::setprecision(17) << "TruthOverride event=" << m_event
              << " track=" << m_trackIndex << " interval=" << interval << " g4Track=" << match.g4TrackID
              << " z=" << truth.retainedFraction << " b=" << b << " addedLossVariance=0"
              << " steps=" << truth.firstStepNumber << ':' << truth.startHookFraction
              << "->" << truth.lastStepNumber << ':' << truth.endHookFraction << endmsg;
        }
        m_truthOverrideStatus = 1;
      }
      const auto seedIndices = adapter.seedHitIndices(hits);
      m_seedHitIndices.assign(seedIndices.begin(), seedIndices.end());
      if (m_verbose) info() << "event=" << m_event << " track=" << m_trackIndex
          << " seed=" << m_seedSelectionName << " ordered-hit indices="
          << seedIndices[0] << ',' << seedIndices[1] << ',' << seedIndices[2] << endmsg;
      const auto paired = fitter.fit(hits, settings);
      const auto& fit=paired.rts;
      const auto& inward=paired.backward;
      if (!std::isfinite(fit.ip.omega) || fit.ip.omega == 0)
        throw std::runtime_error("Invalid IP curvature");
      m_fitPt = std::abs(m_bz * 2.99792458e-4 / fit.ip.omega);
      if (!std::isfinite(inward.ip.omega) || inward.ip.omega==0) throw std::runtime_error("Invalid backward IP curvature");
      m_backwardPt=std::abs(m_bz*2.99792458e-4/inward.ip.omega);
      m_backwardTotalChi2=std::accumulate(inward.backwardChi2.begin(),inward.backwardChi2.end(),0.);
      m_backwardSeedForwardChi2=inward.chi2;
      m_smoothedTotalChi2=fit.smoothedTotalChi2;m_smoothedSeedChi2=fit.smoothedSeedChi2;
      m_smoothedChi2=fit.smoothedChi2;m_smoothedMeasurementChi2=fit.smoothedMeasurementChi2;
      m_smoothedProcessChi2=fit.smoothedProcessChi2;m_smoothedNativeChi2=fit.smoothedNativeMeasurementChi2;
      m_smoothedChi2Status=fit.smoothedChi2Status;m_smoothedChi2Error=fit.smoothedChi2Error;
      if(m_smoothedChi2Status!=1) warning()<<"Smoothed chi2 unavailable: "<<m_smoothedChi2Error<<endmsg;
      for(const auto& loss:inward.breakpoints) {
        m_backwardLoss.push_back(loss.fittedLogLoss);m_backwardLossVariance.push_back(loss.fittedVariance);
      }
      m_backwardIterationPt=inward.iterationInverseAbsOmega;
      for(auto& pt:m_backwardIterationPt) pt*=std::abs(m_bz*2.99792458e-4);
      m_backwardIterationLoss=inward.iterationLoss;m_backwardIterationVariance=inward.iterationLossVariance;
      m_backwardIterationNorm=inward.iterationStepNorm;m_backwardIterationChi2=inward.iterationLinearizedChi2;
      m_backwardIterations=m_backwardIterationPt.empty()?1:m_backwardIterationPt.size();
      m_backwardIterationStatus=inward.iterationStatus;m_backwardIterationError=inward.iterationError;
      m_onePassPt=fit.iterationInverseAbsOmega.empty() ? m_fitPt : std::abs(m_bz*2.99792458e-4/fit.onePassIP.omega);
      m_iterationPt=fit.iterationInverseAbsOmega;
      for(auto& value:m_iterationPt) value*=std::abs(m_bz*2.99792458e-4);
      m_iterationLoss=fit.iterationLoss;m_iterationLossVariance=fit.iterationLossVariance;
      m_iterationNorm=fit.iterationStepNorm;m_iterationChi2=fit.iterationLinearizedChi2;
      m_iterations=m_iterationPt.empty()?1:m_iterationPt.size();
      m_iterationStatus=fit.iterationStatus;m_iterationError=fit.iterationError;
      if(m_verbose)
        for(std::size_t j=0;j<m_iterationPt.size();++j)
          info()<<std::setprecision(17)<<"iteration="<<j+1<<" pt="<<m_iterationPt[j]
                <<" b="<<m_iterationLoss[j]<<" variance="<<m_iterationLossVariance[j]
                <<" normalizedStep="<<m_iterationNorm[j]<<" affineChi2="<<m_iterationChi2[j]<<endmsg;
      if(m_iterationStatus==-1) warning()<<"Relinearization stopped; last complete pass retained: "<<m_iterationError<<endmsg;
      m_fitChi2 = fit.chi2;
      if (m_verifyReference) {
        const auto reference = adapter.referenceKF(hits, m_seedScale,false);
        const auto backwardReference = adapter.referenceKF(hits,m_seedScale,true);
        m_backwardReferencePt=std::abs(m_bz*2.99792458e-4/backwardReference.omega);
        m_referencePt = std::abs(m_bz * 2.99792458e-4 / reference.omega);
        if (m_intervals.value().empty() &&
            (std::abs(m_fitPt / m_referencePt - 1) > 1.e-4 ||
             (m_backwardSeedScale == 1.0 &&
              std::abs(m_backwardPt / m_backwardReferencePt - 1) > 1.e-4)))
          throw std::runtime_error("Empty-breakpoint/native-KF pT regression failed");
        info() << "event=" << m_event << " track=" << m_trackIndex
               << " breakpointPt=" << m_fitPt << " referenceKFPt=" << m_referencePt << endmsg;
      }
      m_localChi2 = fit.localChi2;
      m_persistentHits = fit.persistentHits;
      const auto appendMatrix = [](std::vector<double>& values, const TMatrixD& matrix) {
        for (int row = 0; row < matrix.GetNrows(); ++row)
          for (int col = 0; col < matrix.GetNcols(); ++col) values.push_back(matrix(row, col));
      };
      for (std::size_t j = 0; j < fit.persistentHits.size(); ++j) {
        appendMatrix(m_sixPredictedMean, fit.persistentPredicted[j].mean);
        appendMatrix(m_sixPredictedCov, fit.persistentPredicted[j].covariance);
        appendMatrix(m_sixFilteredMean, fit.persistentFiltered[j].mean);
        appendMatrix(m_sixFilteredCov, fit.persistentFiltered[j].covariance);
        appendMatrix(m_sixSmoothedMean, fit.persistentSmoothed[j].mean);
        appendMatrix(m_sixSmoothedCov, fit.persistentSmoothed[j].covariance);
        appendMatrix(m_sixTransport, fit.persistentTransport[j]);
        appendMatrix(m_sixNoise, fit.persistentNoise[j]);
        if (m_verbose) {
          for (const auto& named : std::vector<std::pair<const char*, const breakpoint::LossTrackState*>>{
              {"predicted6D", &fit.persistentPredicted[j]},
              {"filtered6D", &fit.persistentFiltered[j]},
              {"smoothed6D", &fit.persistentSmoothed[j]}}) {
            std::ostringstream dump;
            dump << std::setprecision(17) << named.first << " owner=" << m_intervals.value().front()
                 << " hit=" << fit.persistentHits[j] << " mean=[";
            for (int row = 0; row < 6; ++row) dump << named.second->mean(row, 0) << ' ';
            dump << "] covariance(row-major)=[";
            for (int row = 0; row < 6; ++row)
              for (int col = 0; col < 6; ++col) dump << named.second->covariance(row, col) << ' ';
            dump << ']';
            info() << dump.str() << endmsg;
          }
        }
      }
      m_backwardChi2 = inward.backwardChi2;
      for (std::size_t i = 0; i < hits.size(); ++i) {
        m_filteredKappa.push_back(fit.filtered[i].mean(2, 0));
        m_filteredVariance.push_back(fit.filtered[i].covariance(2, 2));
        if (!fit.smoothed.empty()) {
          m_smoothedKappa.push_back(fit.smoothed[i].mean(2, 0));
          m_smoothedVariance.push_back(fit.smoothed[i].covariance(2, 2));
        }
        if (!inward.backwardFiltered.empty()) {
          m_backwardKappa.push_back(inward.backwardFiltered[i].mean(2, 0));
          m_backwardVariance.push_back(inward.backwardFiltered[i].covariance(2, 2));
          m_backwardPredictedKappa.push_back(inward.backwardPredicted[i].mean(2, 0));
          m_backwardPredictedVariance.push_back(inward.backwardPredicted[i].covariance(2, 2));
        }
        if (m_verbose) info() << "event=" << m_event << " track=" << m_trackIndex
            << " hit=" << i << " r=" << m_hitR[i] << " localChi2=" << m_localChi2[i]
            << " filtered=" << m_filteredKappa.back() << " endpoint=" << fit.endpoint[i].mean(2, 0)
            << " varFiltered=" << m_filteredVariance.back()
            << " varEndpoint=" << fit.endpoint[i].covariance(2, 2) << endmsg;
        if (m_verbose) {
          // Complete state/covariance dumps make selected-interval mechanics
          // auditable without reducing them to only a curvature error bar.
          std::vector<std::pair<const char*, const breakpoint::TrackState*>> states{
              {"predicted", &fit.predicted[i]}, {"filtered", &fit.filtered[i]}};
          if (!fit.smoothed.empty()) states.push_back({"smoothed", &fit.smoothed[i]});
          if (!inward.backwardFiltered.empty()) {
            states.push_back({"backward_predicted", &inward.backwardPredicted[i]});
            states.push_back({"backward_filtered", &inward.backwardFiltered[i]});
          }
          for (const auto& named : states) {
            std::ostringstream dump;
            dump << std::setprecision(17) << named.first << " hit=" << i << " mean=[";
            for (int row = 0; row < 5; ++row) dump << named.second->mean(row, 0) << ' ';
            dump << "] covariance(row-major)=[";
            for (int row = 0; row < 5; ++row)
              for (int col = 0; col < 5; ++col) dump << named.second->covariance(row, col) << ' ';
            dump << ']';
            info() << dump.str() << endmsg;
          }
        }
      }
      for (const auto& loss : fit.breakpoints) {
        m_breakpointIndex.push_back(loss.index);
        m_priorLoss.push_back(loss.priorLogLoss);
        m_localLoss.push_back(loss.localLogLoss);
        m_localVariance.push_back(loss.localVariance);
        m_fittedLoss.push_back(loss.fittedLogLoss);
        m_lossVariance.push_back(loss.fittedVariance);
        m_closure.push_back(loss.covarianceClosure);
        if (m_verbose) info() << "breakpoint=" << loss.index << " prior=" << loss.priorLogLoss
            << " local=" << loss.localLogLoss << " fitted=" << loss.fittedLogLoss
            << " variance=" << loss.fittedVariance << " closure=" << loss.covarianceClosure << endmsg;
      }
      m_rtsLoss = m_fittedLoss;
      m_rtsLossVariance = m_lossVariance;
      outputIndices->push_back(output->size());
      backwardOutputIndices->push_back(backwardOutput->size());
      auto result = output->create();
      result.addToTrackStates(fit.ip);
      result.addToTrackStates(breakpoint::toEDM(fit.endpoint.front(), m_bz, 2));
      result.addToTrackStates(breakpoint::toEDM(fit.endpoint.back(), m_bz, 3));
      result.setChi2(fit.smoothedTotalChi2);
      // Dimension bookkeeping only; fitted-loss priors preclude assuming a
      // calibrated chi2 distribution. The flat tuple labels all three scores.
      result.setNdf(fit.measurementDimensions - 5);
      for (auto hit : hits) result.addToTrackerHits(hit);
      auto backwardTrack=backwardOutput->create();
      backwardTrack.addToTrackStates(inward.ip);
      backwardTrack.addToTrackStates(breakpoint::toEDM(inward.endpoint.front(),m_bz,2));
      backwardTrack.addToTrackStates(breakpoint::toEDM(inward.endpoint.back(),m_bz,3));
      backwardTrack.setChi2(m_backwardTotalChi2);
      backwardTrack.setNdf(fit.measurementDimensions-5); // bookkeeping, not calibrated
      for(auto hit:hits) backwardTrack.addToTrackerHits(hit);
      m_fitStatus = 1;
    } catch (const std::exception& exception) {
      warning() << "event=" << m_event << " track=" << m_trackIndex << ": " << exception.what() << endmsg;
      outputIndices->push_back(-1);
      backwardOutputIndices->push_back(-1);
    }
    statuses->push_back(m_fitStatus);
    m_tree->Fill();
  }
  return StatusCode::SUCCESS;
}

StatusCode RecBreakpoint::finalize() {
  if (m_file && !m_file->IsZombie()) {
    m_file->cd();
    m_tree->Write();
    m_file->Close();
  }
  return Algorithm::finalize();
}
