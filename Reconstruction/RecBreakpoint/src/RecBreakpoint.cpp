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
#include <optional>

namespace {
enum CaloFamily {
  InputKF, OrdinaryRTS, OrdinaryBackward, FreeRTS, FreeBackward,
  BeamFreeRTS, BeamFreeBackward, TruthRTS, TruthBackward, DiffuseRTS
};
constexpr const char* caloPrefixes[] = {
  "kf_", "rts_", "backward_", "free_loss_rts_", "free_loss_backward_",
  "beam_guided_free_loss_rts_", "beam_guided_free_loss_backward_",
  "truth_override_rts_", "truth_override_backward_", "diffuse_augmented_rts_"
};
// Every endpoint family uses identical EDM representation and hit references.
int publishTrack(edm4hep::TrackCollection& output, const breakpoint::FitResult& fit,
                 const std::vector<edm4hep::TrackerHit>& hits, double bz, double chi2,
                 const breakpoint::KalmanAdapter& adapter,
                 breakpoint::CaloStateTuple& caloTuple) {
  const int index = output.size();
  auto track = output.create();
  track.addToTrackStates(fit.ip);
  track.addToTrackStates(breakpoint::toEDM(fit.endpoint.front(), bz, 2));
  track.addToTrackStates(breakpoint::toEDM(fit.endpoint.back(), bz, 3));
  // A calorimeter extrapolation is diagnostic/publication only. Its failure
  // must never discard or alter the fitted IP and hit states.
  try {
    const auto calo = adapter.propagateToCalorimeter(fit.endpoint.back(), hits.back());
    caloTuple.assign(calo, bz, 1);
    track.addToTrackStates(calo);
  } catch (const std::exception& error) {
    caloTuple.fail(error.what());
  }
  track.setChi2(chi2);
  track.setNdf(fit.measurementDimensions - 5 - fit.freeLossParameterCount); // bookkeeping, not calibrated
  for (auto hit : hits) track.addToTrackerHits(hit);
  return index;
}
}

DECLARE_COMPONENT(RecBreakpoint)

RecBreakpoint::RecBreakpoint(const std::string& name, ISvcLocator* locator)
    : Algorithm(name, locator) {
  declareProperty("InputTracks", m_input, "Tracks whose reconstructed hits are refitted");
  declareProperty("OutputTracks", m_output, "Successful refitted tracks");
  declareProperty("OutputTracksBackwardFilter", m_backwardOutput, "Parallel backward-filter tracks");
  declareProperty("OutputTracksFreeLossRTS", m_freeRTSOutput, "Free-loss RTS; ordinary copy when off, input KF copy on failure");
  declareProperty("OutputTracksFreeLossBackwardFilter", m_freeBackwardOutput, "Free-loss backward; ordinary copy when off, input KF copy on failure");
  declareProperty("OutputTracksBeamGuidedFreeLossRTS", m_beamFreeRTSOutput,
                  "Separate beam-objective free-loss RTS; free-loss copy when disabled, input KF on failure");
  declareProperty("OutputTracksBeamGuidedFreeLossBackwardFilter", m_beamFreeBackwardOutput,
                  "Separate beam-objective free-loss backward endpoint");
  declareProperty("OutputTracksTruthOverrideRTS", m_truthRTSOutput, "Oracle RTS, or ordinary RTS copy when disabled");
  declareProperty("OutputTracksDiffuseAugmentedRTS", m_diffuseRTSOutput,
                  "Exact-diffuse augmented RTS; ordinary copy when disabled or no interval");
  declareProperty("OutputTracksTruthOverrideBackwardFilter", m_truthBackwardOutput, "Oracle backward filter, or ordinary copy when disabled");
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
  m_intervalSelectionName = m_intervalSelection.value();
  if (m_intervalSelectionName == "Auto") {
    error() << "IntervalSelectionMode=Auto is reserved: the reconstruction-based interval finder is not implemented."
            << " Use Truth for this diagnostic or Manual for a fixed list." << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_intervalSelectionName != "Truth" && m_intervalSelectionName != "Manual") {
    error() << "IntervalSelectionMode must be Truth, Manual, or reserved Auto" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_intervalSelectionName != "Manual" && !m_intervals.value().empty()) {
    error() << "BreakpointIntervals is a Manual-only list; clear it for Truth selection" << endmsg;
    return StatusCode::FAILURE;
  }
  if ((m_lossStateModeName != "Persistent6D" && m_lossStateModeName != "LocalMarginal") ||
      (m_lossStateModeName == "Persistent6D" && m_intervals.value().size() > 1)) {
    error() << "LossStateMode must be LocalMarginal or Persistent6D (at most one breakpoint)."
            << " Use TruthOverride=true for the additional oracle pair, not LossStateMode=TruthOverride." << endmsg;
    return StatusCode::FAILURE;
  }
  const std::set<DataObjID> outputNames{m_output.fullKey(), m_backwardOutput.fullKey(),
      m_truthRTSOutput.fullKey(), m_truthBackwardOutput.fullKey(),
      m_freeRTSOutput.fullKey(), m_freeBackwardOutput.fullKey(),
      m_beamFreeRTSOutput.fullKey(), m_beamFreeBackwardOutput.fullKey(),
      m_diffuseRTSOutput.fullKey()};
  if (outputNames.size() != 9 || outputNames.count(m_input.fullKey())) {
    error() << "The nine output track collections must differ from each other and the input" << endmsg;
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
  if (!std::isfinite(m_meanLoss.value()) || m_meanLoss < 0 || m_meanLoss > 5 ||
      !std::isfinite(m_sigmaLoss.value()) || m_sigmaLoss <= 0) {
    error() << "MeanLogLoss must be finite in [0,5] and SigmaLogLoss finite/positive" << endmsg;
    return StatusCode::FAILURE;
  }
  if (!std::isfinite(m_freeLossMax.value()) || m_freeLossMax <= 0 || m_freeLossMax > 5 ||
      m_freeLossMaxCalls < 1 || !std::isfinite(m_freeLossTolerance.value()) || m_freeLossTolerance <= 0) {
    error() << "FreeLossMaxLogLoss must be in (0,5]; FreeLossMaxCallsPerStart and FreeLossTolerance must be positive" << endmsg;
    return StatusCode::FAILURE;
  }
  if (!std::isfinite(m_beamSpotX.value()) || !std::isfinite(m_beamSpotY.value()) ||
      !std::isfinite(m_beamSpotSigmaX.value()) || m_beamSpotSigmaX <= 0 ||
      !std::isfinite(m_beamSpotSigmaY.value()) || m_beamSpotSigmaY <= 0) {
    error() << "BeamSpotX/Y must be finite and BeamSpotSigmaX/Y finite/positive" << endmsg;
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
  for (std::size_t i = 0; i < m_caloStates.size(); ++i)
    m_caloStates[i].book(*m_tree, caloPrefixes[i]);
  m_freeLossTuple.book(*m_tree);
  m_beamFreeLossTuple.book(*m_tree, "beam_guided_free_loss_");
  m_ordinaryLikelihood.book(*m_tree, "ordinary_likelihood_");
  m_freeLikelihood.book(*m_tree, "free_loss_likelihood_");
  m_beamFreeLikelihood.book(*m_tree, "beam_guided_free_loss_likelihood_");
  m_truthLikelihood.book(*m_tree, "truth_override_likelihood_");
  m_freeLossTracks.book(*m_tree, "free_loss_");
  m_beamFreeLossTracks.book(*m_tree, "beam_guided_free_loss_");
  m_tree->Branch("diffuse_augmented_status", &m_diffuseStatus);
  m_tree->Branch("diffuse_augmented_index", &m_diffuseIndex);
  m_tree->Branch("diffuse_augmented_error", &m_diffuseError);
  m_tree->Branch("diffuse_augmented_pt", &m_diffusePt);
  m_tree->Branch("diffuse_augmented_fitted_log_loss", &m_diffuseB);
  m_tree->Branch("diffuse_augmented_fitted_log_loss_variance", &m_diffuseBVariance);
  m_tree->Branch("diffuse_augmented_ip_parameters", &m_diffuseIPParameters);
  m_tree->Branch("diffuse_augmented_ip_covariance", &m_diffuseIPCovariance);
  m_tree->Branch("diffuse_augmented_local_chi2", &m_diffuseLocalChi2);
  m_tree->Branch("diffuse_augmented_filtered_log_loss", &m_diffuseFilteredB);
  m_tree->Branch("diffuse_augmented_smoothed_log_loss", &m_diffuseSmoothedB);
  m_tree->Branch("diffuse_augmented_hit_index", &m_diffuseHitIndex);
  m_tree->Branch("diffuse_augmented_predicted_unresolved", &m_diffusePredictedUnresolved);
  m_tree->Branch("diffuse_augmented_filtered_unresolved", &m_diffuseFilteredUnresolved);
  m_tree->Branch("diffuse_augmented_predicted_mean", &m_diffusePredictedMean);
  m_tree->Branch("diffuse_augmented_predicted_covariance", &m_diffusePredictedCovariance);
  m_tree->Branch("diffuse_augmented_filtered_mean", &m_diffuseFilteredMean);
  m_tree->Branch("diffuse_augmented_filtered_covariance", &m_diffuseFilteredCovariance);
  m_tree->Branch("diffuse_augmented_smoothed_mean", &m_diffuseSmoothedMean);
  m_tree->Branch("diffuse_augmented_smoothed_covariance", &m_diffuseSmoothedCovariance);
  m_tree->Branch("diffuse_augmented_transport", &m_diffuseTransport);
  m_tree->Branch("diffuse_augmented_process_noise", &m_diffuseNoise);
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
  m_tree->Branch("loss_state_mode", &m_lossStateModeName);
  m_tree->Branch("interval_selection_mode", &m_intervalSelectionName);
  m_tree->Branch("interval_selection_status", &m_intervalSelectionStatus);
  m_tree->Branch("interval_selection_error", &m_intervalSelectionError);
  m_tree->Branch("selected_breakpoint_interval", &m_selectedIntervals);
  m_tree->Branch("interval_selection_truth_ebrem_loss", &m_intervalTruthLoss);
  m_tree->Branch("interval_selection_truth_retained_fraction", &m_intervalTruthZ);
  m_tree->Branch("interval_selection_truth_g4_track_id", &m_intervalTruthTrack);
  m_tree->Branch("interval_selection_truth_max_endpoint_distance", &m_intervalTruthDistance);
  m_tree->Branch("truth_override_status", &m_truthOverrideStatus);
  m_tree->Branch("truth_override_result_status", &m_truthResultCode);
  m_tree->Branch("truth_override_loss_treatment", &m_truthLossTreatment);
  m_tree->Branch("truth_override_prior_sigma_log_loss", &m_truthPriorSigma);
  m_tree->Branch("truth_override_rts_fitted_log_loss", &m_truthRTSLoss);
  m_tree->Branch("truth_override_rts_fitted_log_loss_variance", &m_truthRTSLossVariance);
  m_tree->Branch("truth_override_backward_fitted_log_loss", &m_truthBackwardLoss);
  m_tree->Branch("truth_override_backward_fitted_log_loss_variance", &m_truthBackwardLossVariance);
  m_tree->Branch("truth_override_rts_pt", &m_truthRTSPt);
  m_tree->Branch("truth_override_backward_pt", &m_truthBackwardPt);
  m_tree->Branch("truth_override_forward_chi2", &m_truthForwardChi2);
  m_tree->Branch("truth_override_backward_chi2", &m_truthBackwardChi2);
  m_tree->Branch("truth_override_smoothed_chi2", &m_truthSmoothedChi2);
  m_tree->Branch("truth_override_smoothed_chi2_status", &m_truthSmoothedStatus);
  m_tree->Branch("truth_override_smoothed_chi2_error", &m_truthSmoothedError);
  m_tree->Branch("truth_override_forward_local_chi2", &m_truthForwardLocal);
  m_tree->Branch("truth_override_backward_local_chi2", &m_truthBackwardLocal);
  m_tree->Branch("truth_override_smoothed_local_chi2", &m_truthSmoothedLocal);
  m_tree->Branch("truth_override_smoothed_measurement_chi2", &m_truthSmoothedMeasurement);
  m_tree->Branch("truth_override_smoothed_process_chi2", &m_truthSmoothedProcess);
  m_tree->Branch("truth_override_smoothed_native_measurement_chi2", &m_truthSmoothedNative);
  m_tree->Branch("truth_override_smoothed_seed_chi2", &m_truthSmoothedSeed);
  m_tree->Branch("truth_override_rts_ip_parameters", &m_truthRTSParameters);
  m_tree->Branch("truth_override_rts_ip_covariance", &m_truthRTSCovariance);
  m_tree->Branch("truth_override_backward_ip_parameters", &m_truthBackwardParameters);
  m_tree->Branch("truth_override_backward_ip_covariance", &m_truthBackwardCovariance);
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
  info() << "Outward KF + parallel RTS and BackwardFilter; interval selection=" << m_intervalSelectionName
         << "; seed=" << m_seedSelectionName
         << "; Bz=" << m_bz << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecBreakpoint::execute() {
  ++m_event;
  auto* output = m_output.createAndPut();
  auto* backwardOutput = m_backwardOutput.createAndPut();
  auto* freeRTSOutput = m_freeRTSOutput.createAndPut();
  auto* freeBackwardOutput = m_freeBackwardOutput.createAndPut();
  auto* freeResultStatuses = m_freeResultStatus.createAndPut();
  auto* freeRTSIndices = m_freeRTSIndex.createAndPut();
  auto* freeBackwardIndices = m_freeBackwardIndex.createAndPut();
  auto* beamFreeRTSOutput = m_beamFreeRTSOutput.createAndPut();
  auto* beamFreeBackwardOutput = m_beamFreeBackwardOutput.createAndPut();
  auto* beamFreeResultStatuses = m_beamFreeResultStatus.createAndPut();
  auto* beamFreeRTSIndices = m_beamFreeRTSIndex.createAndPut();
  auto* beamFreeBackwardIndices = m_beamFreeBackwardIndex.createAndPut();
  auto* truthRTSOutput = m_truthRTSOutput.createAndPut();
  auto* diffuseRTSOutput = m_diffuseRTSOutput.createAndPut();
  auto* diffuseStatuses = m_diffuseStatusOutput.createAndPut();
  auto* diffuseIndices = m_diffuseIndexOutput.createAndPut();
  auto* truthBackwardOutput = m_truthBackwardOutput.createAndPut();
  auto* truthResultStatuses = m_truthResultStatus.createAndPut();
  auto* truthRTSIndices = m_truthRTSIndex.createAndPut();
  auto* truthBackwardIndices = m_truthBackwardIndex.createAndPut();
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
  settings.captureGaussianModel = true; // passive score for ordinary and truth RTS
  const breakpoint::FreeLossSettings freeLossControls{m_freeLossMax.value(),
      static_cast<unsigned>(m_freeLossMaxCalls.value()), m_freeLossTolerance.value(), std::nullopt};
  auto beamFreeLossControls = freeLossControls;
  beamFreeLossControls.beamSpot = breakpoint::BeamSpotSettings{
      m_beamSpotX.value(), m_beamSpotY.value(),
      m_beamSpotSigmaX.value(), m_beamSpotSigmaY.value()};
  const bool needTruthData = selected && (m_intervalSelectionName == "Truth" ||
      (m_enableTruthOverride && !settings.intervals.empty()));
  TruthBHLossEventData truthReader; // event-local maps, released after this event
  bool truthPrepared = false;
  std::string truthEventError;
  if (needTruthData) {
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
    if (!selected) {
      statuses->push_back(0); outputIndices->push_back(-1); backwardOutputIndices->push_back(-1);
      truthResultStatuses->push_back(0); truthRTSIndices->push_back(-1); truthBackwardIndices->push_back(-1);
      freeResultStatuses->push_back(0); freeRTSIndices->push_back(-1); freeBackwardIndices->push_back(-1);
      beamFreeResultStatuses->push_back(0); beamFreeRTSIndices->push_back(-1);
      beamFreeBackwardIndices->push_back(-1);
      diffuseStatuses->push_back(0); diffuseIndices->push_back(-1);
      continue;
    }
    int truthRTSIndex = -1, truthBackwardIndex = -1;
    m_freeLossTuple.reset(m_freeLossFit, freeLossControls, settings.sigmaLogLoss);
    m_beamFreeLossTuple.reset(m_freeLossFit && m_freeLossBeamSpotObjective,
                              beamFreeLossControls, settings.sigmaLogLoss);
    m_ordinaryLikelihood.reset();
    m_freeLikelihood.reset();
    m_beamFreeLikelihood.reset();
    m_truthLikelihood.reset();
    m_freeLossTracks.reset();
    m_beamFreeLossTracks.reset();
    for (auto& calo : m_caloStates) calo.reset();
    m_caloStates[InputKF].assignFromTrack(track, m_bz, 2);
    m_diffuseStatus = 0; m_diffuseIndex = -1; m_diffuseError.clear();
    m_diffusePt = m_diffuseB = m_diffuseBVariance = nan;
    m_diffuseIPParameters.clear(); m_diffuseIPCovariance.clear();
    m_diffuseLocalChi2.clear(); m_diffuseFilteredB.clear(); m_diffuseSmoothedB.clear();
    m_diffuseHitIndex.clear();
    m_diffusePredictedUnresolved.clear(); m_diffuseFilteredUnresolved.clear();
    m_diffusePredictedMean.clear(); m_diffusePredictedCovariance.clear();
    m_diffuseFilteredMean.clear(); m_diffuseFilteredCovariance.clear();
    m_diffuseSmoothedMean.clear(); m_diffuseSmoothedCovariance.clear();
    m_diffuseTransport.clear(); m_diffuseNoise.clear();
    m_truthLossTreatment = "PriorCenter";
    m_truthResultCode = 0;
    m_truthRTSPt = m_truthBackwardPt = m_truthForwardChi2 = m_truthBackwardChi2 = m_truthSmoothedChi2 = nan;
    m_truthSmoothedStatus = 0; m_truthSmoothedError.clear();
    m_truthForwardLocal.clear(); m_truthBackwardLocal.clear(); m_truthSmoothedLocal.clear();
    m_truthSmoothedMeasurement.clear(); m_truthSmoothedProcess.clear(); m_truthSmoothedNative.clear();
    m_truthSmoothedSeed = nan;
    m_truthRTSParameters.clear(); m_truthRTSCovariance.clear();
    m_truthBackwardParameters.clear(); m_truthBackwardCovariance.clear();
    m_truthPriorSigma = settings.sigmaLogLoss;
    m_truthRTSLoss.clear(); m_truthRTSLossVariance.clear();
    m_truthBackwardLoss.clear(); m_truthBackwardLossVariance.clear();
    m_fitStatus = -1;
    settings.intervals = m_intervals.value(); // never reuse the previous track's truth-selected list
    bool needTruthLoss = false;
    m_intervalSelectionStatus = 0; m_intervalSelectionError.clear();
    m_selectedIntervals.clear(); m_intervalTruthLoss.clear(); m_intervalTruthZ.clear();
    m_intervalTruthTrack = -1; m_intervalTruthDistance = nan;
    settings.intervalMeanLogLoss.clear();
    m_truthOverrideStatus = 0; m_truthG4Track = -1; m_truthMaxDistance = nan;
    m_truthOverrideError.clear(); m_truthIntervals.clear(); m_truthZ.clear(); m_truthB.clear();
    m_truthMomentumBefore.clear(); m_truthEbremLoss.clear(); m_truthTX0.clear();
    m_truthFirstStep.clear(); m_truthLastStep.clear();
    m_truthStartFraction.clear(); m_truthEndFraction.clear();
    m_seedHitIndices.clear();
    m_persistentHits.clear();
    m_sixPredictedMean.clear(); m_sixPredictedCov.clear();
    m_sixFilteredMean.clear(); m_sixFilteredCov.clear();
    m_sixSmoothedMean.clear(); m_sixSmoothedCov.clear();
    m_sixTransport.clear(); m_sixNoise.clear();
    m_kfPt = m_fitPt = m_fitChi2 = m_referencePt = nan;
    m_backwardPt=m_backwardTotalChi2=m_smoothedTotalChi2=m_smoothedSeedChi2=nan;
    m_backwardSeedForwardChi2=m_backwardReferencePt=nan;
    m_smoothedChi2Status=0;
    m_smoothedChi2Error.clear();
    m_smoothedChi2.clear();m_smoothedMeasurementChi2.clear();m_smoothedProcessChi2.clear();m_smoothedNativeChi2.clear();
    m_backwardLoss.clear();m_backwardLossVariance.clear();
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
      // One association-driven truth match can serve interval selection and the
      // optional truth-centered fit. Ordinary fitting receives ONLY the indices,
      // never the truth loss center/variance, from Truth interval selection.
      TruthBHLossEventDataMatch match;
      bool matchedTruth = false;
      std::string matchError;
      auto matchTruth = [&]() {
        if (!matchedTruth)
          matchedTruth = truthReader.matchTrack(hits, m_truthEndpointDistance, true, match, matchError);
        return matchedTruth;
      };
      if (m_intervalSelectionName == "Truth") {
        if (!truthPrepared) {
          m_intervalSelectionStatus = -1;
          m_intervalSelectionError = truthEventError;
          throw std::runtime_error("Truth interval input: " + truthEventError);
        }
        if (!matchTruth()) {
          m_intervalSelectionStatus = -2;
          m_intervalSelectionError = matchError;
          throw std::runtime_error("Truth interval association: " + matchError);
        }
        m_intervalTruthTrack = match.g4TrackID;
        m_intervalTruthDistance = match.maxEndpointDistance;
        // One common breakpoint for all endpoint families: choose the largest
        // summed absolute eBrem momentum loss between accepted hit hooks.
        // Equal losses keep the first (innermost) interval. Other losses are
        // deliberately not fitted; the optimizer still receives only an index.
        int largest = -1;
        double largestLoss = 0.;
        for (std::size_t i = 0; i < match.materialIntervals.size(); ++i) {
          if (match.materialIntervals[i].ebremLoss > largestLoss) {
            largest = static_cast<int>(i);
            largestLoss = match.materialIntervals[i].ebremLoss;
          }
        }
        if (largest >= 0) {
          const auto& interval = match.materialIntervals[largest];
          settings.intervals.push_back(largest);
          m_intervalTruthLoss.push_back(interval.ebremLoss);
          m_intervalTruthZ.push_back(interval.retainedFraction);
        }
        m_intervalSelectionStatus = 2;
      } else {
        m_intervalSelectionStatus = 1;
      }
      m_selectedIntervals = settings.intervals;
      needTruthLoss = m_enableTruthOverride && !settings.intervals.empty();
      if (m_verbose) {
        std::ostringstream selection;
        selection << "event=" << m_event << " track=" << m_trackIndex
                  << " intervalSelection=" << m_intervalSelectionName << " intervals=[";
        for (int i : settings.intervals) selection << i << ' ';
        selection << "] ordinary prior b=" << settings.meanLogLoss << " sigma=" << settings.sigmaLogLoss;
        info() << selection.str() << endmsg;
      }
      const auto seedIndices = adapter.seedHitIndices(hits);
      m_seedHitIndices.assign(seedIndices.begin(), seedIndices.end());
      if (m_verbose) info() << "event=" << m_event << " track=" << m_trackIndex
          << " seed=" << m_seedSelectionName << " ordered-hit indices="
          << seedIndices[0] << ',' << seedIndices[1] << ',' << seedIndices[2] << endmsg;
      const auto paired = fitter.fit(hits, settings);
      std::optional<breakpoint::PairedFitResult> freePair;
      std::optional<breakpoint::TrackLikelihoodResult> optimizedLikelihood;
      std::optional<breakpoint::PairedFitResult> beamFreePair;
      std::optional<breakpoint::TrackLikelihoodResult> optimizedBeamLikelihood;
      bool freeKFFallback = false;
      bool beamFreeKFFallback = false;
      if (m_freeLossFit) {
        const breakpoint::FreeLossFitter optimizer(fitter, m_bz);
        auto freeFit = optimizer.fit(hits, settings, freeLossControls);
        m_freeLossTuple.assign(freeFit.diagnostics);
        if (freeFit.diagnostics.status == breakpoint::FreeLossStatus::Applied)
          optimizedLikelihood = freeFit.diagnostics.likelihood;
        freePair = std::move(freeFit.fitted);
        freeKFFallback = freeFit.diagnostics.status == breakpoint::FreeLossStatus::Failed ||
                         freeFit.diagnostics.status == breakpoint::FreeLossStatus::Unsupported;
        if (!freeFit.diagnostics.error.empty())
          warning() << "FreeLossFit: " << freeFit.diagnostics.error << "; free outputs use input KF" << endmsg;
        if (m_verbose) info() << std::setprecision(17) << "FreeLossFit event=" << m_event
            << " track=" << m_trackIndex << " status=" << int(freeFit.diagnostics.status)
            << " prior_mean=" << freeFit.diagnostics.b << " prior_sigma=" << settings.sigmaLogLoss
            << " nll2=" << freeFit.diagnostics.likelihood.nll2
            << " minuit_status=" << freeFit.diagnostics.minuitStatus << endmsg;
        if (m_freeLossBeamSpotObjective) {
          auto beamFit = optimizer.fit(hits, settings, beamFreeLossControls);
          m_beamFreeLossTuple.assign(beamFit.diagnostics);
          if (beamFit.diagnostics.status == breakpoint::FreeLossStatus::Applied)
            optimizedBeamLikelihood = beamFit.diagnostics.likelihood;
          beamFreePair = std::move(beamFit.fitted);
          beamFreeKFFallback = beamFit.diagnostics.status == breakpoint::FreeLossStatus::Failed ||
                               beamFit.diagnostics.status == breakpoint::FreeLossStatus::Unsupported;
          if (!beamFit.diagnostics.error.empty())
            warning() << "Beam-guided FreeLossFit: " << beamFit.diagnostics.error
                      << "; beam-guided outputs use input KF" << endmsg;
          if (m_verbose) info() << std::setprecision(17)
              << "BeamGuidedFreeLossFit event=" << m_event << " track=" << m_trackIndex
              << " status=" << int(beamFit.diagnostics.status)
              << " prior_mean=" << beamFit.diagnostics.b
              << " hit_nll2=" << beamFit.diagnostics.likelihood.nll2
              << " beam_nll2=" << beamFit.diagnostics.beam.nll2
              << " objective_nll2=" << beamFit.diagnostics.objectiveNll2
              << " beam_residual_mm=" << beamFit.diagnostics.beam.residual
              << " beam_variance_mm2=" << beamFit.diagnostics.beam.innovationVariance
              << " minuit_status=" << beamFit.diagnostics.minuitStatus << endmsg;
        }
      }
      const auto& fit=paired.rts;
      const auto& inward=paired.backward;
      m_ordinaryLikelihood.assign(fit);
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
      m_fitChi2 = fit.chi2;
      if (m_verifyReference) {
        const auto reference = adapter.referenceKF(hits, m_seedScale,false);
        const auto backwardReference = adapter.referenceKF(hits,m_seedScale,true);
        m_backwardReferencePt=std::abs(m_bz*2.99792458e-4/backwardReference.omega);
        m_referencePt = std::abs(m_bz * 2.99792458e-4 / reference.omega);
        if (settings.intervals.empty() &&
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
            dump << std::setprecision(17) << named.first << " owner=" << settings.intervals.front()
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
      outputIndices->push_back(publishTrack(*output, fit, hits, m_bz, fit.smoothedTotalChi2,
                                            adapter, m_caloStates[OrdinaryRTS]));
      backwardOutputIndices->push_back(publishTrack(*backwardOutput, inward, hits, m_bz,
                                                    m_backwardTotalChi2, adapter,
                                                    m_caloStates[OrdinaryBackward]));
      m_fitStatus = 1;

      // Independent flat-prior experiment. A failed or underidentified fit
      // never changes the ordinary pair and is explicitly an input-KF copy.
      if (!m_diffuseAugmentedRTS || settings.intervals.empty()) {
        m_diffuseStatus = settings.intervals.empty() ? 1 : 0;
        m_diffuseIndex = publishTrack(*diffuseRTSOutput, fit, hits, m_bz,
                                      fit.smoothedTotalChi2, adapter, m_caloStates[DiffuseRTS]);
        m_diffusePt = m_fitPt;
        m_diffuseIPParameters = {fit.ip.D0, fit.ip.phi, fit.ip.omega, fit.ip.Z0, fit.ip.tanLambda};
        m_diffuseIPCovariance.assign(fit.ip.covMatrix.begin(), fit.ip.covMatrix.end());
      } else try {
        const auto diffuse = fitter.fitDiffuseRTS(hits, settings);
        if (!std::isfinite(diffuse.ip.omega) || diffuse.ip.omega == 0 ||
            diffuse.breakpoints.empty() || !std::isfinite(diffuse.breakpoints.front().fittedVariance))
          throw std::runtime_error("Invalid diffuse endpoint or loss posterior");
        m_diffuseStatus = 2;
        m_diffuseIndex = publishTrack(*diffuseRTSOutput, diffuse, hits, m_bz,
                                      diffuse.chi2, adapter, m_caloStates[DiffuseRTS]);
        m_diffusePt = std::abs(m_bz * 2.99792458e-4 / diffuse.ip.omega);
        m_diffuseB = diffuse.breakpoints.front().fittedLogLoss;
        m_diffuseBVariance = diffuse.breakpoints.front().fittedVariance;
        m_diffuseIPParameters = {diffuse.ip.D0, diffuse.ip.phi, diffuse.ip.omega,
                                 diffuse.ip.Z0, diffuse.ip.tanLambda};
        m_diffuseIPCovariance.assign(diffuse.ip.covMatrix.begin(), diffuse.ip.covMatrix.end());
        m_diffuseLocalChi2 = diffuse.localChi2;
        m_diffuseHitIndex = diffuse.persistentHits;
        m_diffusePredictedUnresolved = diffuse.persistentPredictedDiffuse;
        m_diffuseFilteredUnresolved = diffuse.persistentFilteredDiffuse;
        for (std::size_t j = 0; j < diffuse.persistentHits.size(); ++j) {
          const auto& predicted = diffuse.persistentPredicted[j];
          const auto& filtered = diffuse.persistentFiltered[j];
          const auto& smoothed = diffuse.persistentSmoothed[j];
          m_diffuseFilteredB.push_back(filtered.mean(5, 0));
          m_diffuseSmoothedB.push_back(smoothed.mean(5, 0));
          appendMatrix(m_diffusePredictedMean, predicted.mean);
          appendMatrix(m_diffusePredictedCovariance, predicted.covariance);
          appendMatrix(m_diffuseFilteredMean, filtered.mean);
          appendMatrix(m_diffuseFilteredCovariance, filtered.covariance);
          appendMatrix(m_diffuseSmoothedMean, smoothed.mean);
          appendMatrix(m_diffuseSmoothedCovariance, smoothed.covariance);
          appendMatrix(m_diffuseTransport, diffuse.persistentTransport[j]);
          appendMatrix(m_diffuseNoise, diffuse.persistentNoise[j]);
          if (m_verbose) {
            std::ostringstream dump;
            dump << std::setprecision(17) << "diffuse6D hit=" << diffuse.persistentHits[j]
                 << " filtered=[";
            for (int row = 0; row < 6; ++row) dump << filtered.mean(row, 0) << ' ';
            dump << "] smoothed=[";
            for (int row = 0; row < 6; ++row) dump << smoothed.mean(row, 0) << ' ';
            dump << "] filteredCov=[";
            for (int row = 0; row < 6; ++row)
              for (int col = 0; col < 6; ++col) dump << filtered.covariance(row, col) << ' ';
            dump << "] smoothedCov=[";
            for (int row = 0; row < 6; ++row)
              for (int col = 0; col < 6; ++col) dump << smoothed.covariance(row, col) << ' ';
            info() << dump.str() << endmsg;
          }
        }
        if (m_verbose) info() << "diffuse event=" << m_event << " track=" << m_trackIndex
          << " interval=" << settings.intervals.front() << " b=" << m_diffuseB
          << " variance=" << m_diffuseBVariance << " pt=" << m_diffusePt << endmsg;
      } catch (const std::exception& error) {
        m_diffuseStatus = -1;
        m_diffuseError = error.what();
        m_diffuseIndex = diffuseRTSOutput->size();
        diffuseRTSOutput->push_back(track.clone());
        m_caloStates[DiffuseRTS].assignFromTrack(track, m_bz, 2);
        m_diffusePt = m_kfPt;
        warning() << "DiffuseAugmentedRTS input-KF fallback: " << m_diffuseError << endmsg;
      }

      // Never replace ordinary results. A failed/unsupported optimization
      // copies CompleteTracks itself, not a breakpoint or fresh KF refit.
      const auto& freeResult = freePair ? *freePair : paired;
      if (freeKFFallback) {
        m_freeLikelihood.invalidate("Free-loss output is input KF fallback; no breakpoint likelihood");
        try {
          const int rtsIndex = freeRTSOutput->size();
          const int backwardIndex = freeBackwardOutput->size();
          m_freeLossTracks.assignKF(track, m_bz, rtsIndex, backwardIndex);
          freeRTSOutput->push_back(track.clone());
          freeBackwardOutput->push_back(track.clone());
          m_caloStates[FreeRTS].assignFromTrack(track, m_bz, 2);
          m_caloStates[FreeBackward].assignFromTrack(track, m_bz, 2);
          if (m_verbose) info() << "FreeLoss KF fallback event=" << m_event
              << " track=" << m_trackIndex << " inputPt=" << m_kfPt << endmsg;
        } catch (const std::exception& error) {
          m_freeLossTracks.reset();
          warning() << "Ordinary pair retained; input KF fallback unavailable: "
                    << error.what() << endmsg;
        }
      } else {
        if (freePair) {
          // The accepted Minuit trial already used this exact normalized
          // objective. Do not silently replace it with a different score.
          if (!optimizedLikelihood) m_freeLikelihood.invalidate("Accepted free fit has no likelihood");
          else {
            try { m_freeLikelihood.assign(*optimizedLikelihood); }
            catch (const std::exception& error) { m_freeLikelihood.invalidate(error.what()); }
          }
        } else {
          m_freeLikelihood = m_ordinaryLikelihood; // free fit off or no interval
        }
        const double freeBackwardChi2 = std::accumulate(freeResult.backward.backwardChi2.begin(),
                                                       freeResult.backward.backwardChi2.end(), 0.);
        const int freeRTSIndex = publishTrack(*freeRTSOutput, freeResult.rts, hits, m_bz,
                                              freeResult.rts.smoothedTotalChi2, adapter,
                                              m_caloStates[FreeRTS]);
        const int freeBackwardIndex = publishTrack(*freeBackwardOutput, freeResult.backward, hits,
                                                   m_bz, freeBackwardChi2, adapter,
                                                   m_caloStates[FreeBackward]);
        m_freeLossTracks.assign(freeResult, m_bz, freePair ? 2 : 1, freeRTSIndex, freeBackwardIndex);
      }
      if (m_verbose && !freeKFFallback) {
        for (std::size_t i = 0; i < hits.size(); ++i) {
          for (const auto& named : std::vector<std::pair<const char*, const breakpoint::TrackState*>>{
              {"free_predicted", &freeResult.rts.predicted[i]},
              {"free_filtered", &freeResult.rts.filtered[i]},
              {"free_smoothed", &freeResult.rts.endpoint[i]},
              {"free_backward_predicted", &freeResult.backward.backwardPredicted[i]},
              {"free_backward_filtered", &freeResult.backward.endpoint[i]}}) {
            std::ostringstream dump;
            dump << std::setprecision(17) << named.first << " hit=" << i << " mean=[";
            for (int row = 0; row < 5; ++row) dump << named.second->mean(row, 0) << ' ';
            dump << "] covariance(row-major)=[";
            for (int row = 0; row < 5; ++row)
              for (int column = 0; column < 5; ++column)
                dump << named.second->covariance(row, column) << ' ';
            dump << ']';
            info() << dump.str() << endmsg;
          }
        }
      }

      // The beam is an extra scalar objective term ONLY. The accepted center
      // is refitted with the same detector hits; no beam Kalman update enters
      // either endpoint or its covariance. When disabled, copy the base free
      // result rather than silently choosing a third fit.
      const auto& beamResult = beamFreePair ? *beamFreePair : freeResult;
      const bool beamFallback = m_freeLossBeamSpotObjective ? beamFreeKFFallback : freeKFFallback;
      if (beamFallback) {
        m_beamFreeLikelihood.invalidate("Beam-guided output is input KF fallback; no breakpoint likelihood");
        try {
          const int rtsIndex = beamFreeRTSOutput->size();
          const int backwardIndex = beamFreeBackwardOutput->size();
          m_beamFreeLossTracks.assignKF(track, m_bz, rtsIndex, backwardIndex);
          beamFreeRTSOutput->push_back(track.clone());
          beamFreeBackwardOutput->push_back(track.clone());
          m_caloStates[BeamFreeRTS].assignFromTrack(track, m_bz, 2);
          m_caloStates[BeamFreeBackward].assignFromTrack(track, m_bz, 2);
        } catch (const std::exception& error) {
          m_beamFreeLossTracks.reset();
          warning() << "Beam-guided input KF fallback unavailable: " << error.what() << endmsg;
        }
      } else {
        if (beamFreePair) {
          if (!optimizedBeamLikelihood)
            m_beamFreeLikelihood.invalidate("Accepted beam-guided fit has no hit likelihood");
          else {
            try { m_beamFreeLikelihood.assign(*optimizedBeamLikelihood); }
            catch (const std::exception& error) { m_beamFreeLikelihood.invalidate(error.what()); }
          }
        } else if (m_freeLossBeamSpotObjective) {
          m_beamFreeLikelihood = m_ordinaryLikelihood; // no selected interval
        } else {
          m_beamFreeLikelihood = m_freeLikelihood; // exact base free-loss copy
        }
        const double backwardChi2 = std::accumulate(beamResult.backward.backwardChi2.begin(),
                                                    beamResult.backward.backwardChi2.end(), 0.);
        const int rtsIndex = publishTrack(*beamFreeRTSOutput, beamResult.rts, hits, m_bz,
                                          beamResult.rts.smoothedTotalChi2, adapter,
                                          m_caloStates[BeamFreeRTS]);
        const int backwardIndex = publishTrack(*beamFreeBackwardOutput, beamResult.backward,
                                               hits, m_bz, backwardChi2, adapter,
                                               m_caloStates[BeamFreeBackward]);
        m_beamFreeLossTracks.assign(beamResult, m_bz, beamFreePair ? 2 : 1,
                                   rtsIndex, backwardIndex);
      }
      if (m_verbose && !beamFallback) {
        for (std::size_t i = 0; i < hits.size(); ++i) {
          for (const auto& named : std::vector<std::pair<const char*, const breakpoint::TrackState*>>{
              {"beam_guided_predicted", &beamResult.rts.predicted[i]},
              {"beam_guided_filtered", &beamResult.rts.filtered[i]},
              {"beam_guided_smoothed", &beamResult.rts.endpoint[i]},
              {"beam_guided_backward_predicted", &beamResult.backward.backwardPredicted[i]},
              {"beam_guided_backward_filtered", &beamResult.backward.endpoint[i]}}) {
            std::ostringstream dump;
            dump << std::setprecision(17) << named.first << " hit=" << i << " mean=[";
            for (int row = 0; row < 5; ++row) dump << named.second->mean(row, 0) << ' ';
            dump << "] covariance(row-major)=[";
            for (int row = 0; row < 5; ++row)
              for (int column = 0; column < 5; ++column)
                dump << named.second->covariance(row, column) << ' ';
            dump << ']';
            info() << dump.str() << endmsg;
          }
        }
      }

      // The ordinary pair is complete before assigning truth prior centers. Failure of the
      // diagnostic oracle must not discard or silently replace ordinary tracks.
      try {
        auto oracleSettings = settings;
        // Truth-prior results remain independent of the optional free pair.
        // When disabled, they copy the ordinary pair, never the optimized one.
        // Only the per-interval b prior centers differ. Keep the same sigma_b,
        // loss-state implementation, seeds and native one-pass updates.
        if (needTruthLoss) {
          if (!truthPrepared) {
            m_truthOverrideStatus = -1;
            m_truthOverrideError = truthEventError;
            throw std::runtime_error("TruthOverride event data: " + truthEventError);
          }
          const bool matched = matchTruth();
          m_truthOverrideError = matchError;
          m_truthG4Track = match.g4TrackID;
          m_truthMaxDistance = match.maxEndpointDistance;
          if (!matched) {
            m_truthOverrideStatus = -2;
            throw std::runtime_error("TruthOverride track association: " + m_truthOverrideError);
          }
          for (int interval : oracleSettings.intervals) {
            if (interval < 0 || interval >= static_cast<int>(match.materialIntervals.size())) {
              m_truthOverrideStatus = -3;
              m_truthOverrideError = "Configured interval is outside the matched track";
              throw std::runtime_error(m_truthOverrideError);
            }
          }
          for (int interval : oracleSettings.intervals) {
            const auto& truth = match.materialIntervals[interval];
            const double b = -std::log(truth.retainedFraction);
            oracleSettings.intervalMeanLogLoss.emplace(interval, b);
            m_truthIntervals.push_back(interval); m_truthZ.push_back(truth.retainedFraction);
            m_truthB.push_back(b); m_truthMomentumBefore.push_back(truth.momentumBefore);
            m_truthEbremLoss.push_back(truth.ebremLoss); m_truthTX0.push_back(truth.truthTX0);
            m_truthFirstStep.push_back(truth.firstStepNumber); m_truthLastStep.push_back(truth.lastStepNumber);
            m_truthStartFraction.push_back(truth.startHookFraction); m_truthEndFraction.push_back(truth.endHookFraction);
            if (m_verbose) info() << std::setprecision(17) << "TruthOverride event=" << m_event
                << " track=" << m_trackIndex << " interval=" << interval << " g4Track=" << match.g4TrackID
                << " z=" << truth.retainedFraction << " prior_b=" << b
                << " prior_sigma_b=" << oracleSettings.sigmaLogLoss
                << " steps=" << truth.firstStepNumber << ':' << truth.startHookFraction
                << "->" << truth.lastStepNumber << ':' << truth.endHookFraction << endmsg;
          }
          m_truthOverrideStatus = 1;
        }

        std::unique_ptr<breakpoint::PairedFitResult> oracle;
        if (needTruthLoss)
          oracle = std::make_unique<breakpoint::PairedFitResult>(fitter.fit(hits, oracleSettings));
        const auto& extraPair = oracle ? *oracle : paired;
        const auto& truthRTS = extraPair.rts;
        const auto& truthBackward = extraPair.backward;
        if (oracle) m_truthLikelihood.assign(truthRTS);
        else m_truthLikelihood = m_ordinaryLikelihood;
        for (const auto* candidate : {&truthRTS, &truthBackward})
          if (!std::isfinite(candidate->ip.omega) || candidate->ip.omega == 0)
            throw std::runtime_error("Invalid truth-override endpoint curvature");

        // Use exactly the same endpoint representation and chi2 conventions.
        // With the switch off these are copies of the already calculated pair,
        // not a rerun with slightly different floating-point results.
        m_truthRTSPt = std::abs(m_bz * 2.99792458e-4 / truthRTS.ip.omega);
        m_truthBackwardPt = std::abs(m_bz * 2.99792458e-4 / truthBackward.ip.omega);
        m_truthForwardChi2 = truthRTS.chi2;
        m_truthBackwardChi2 = std::accumulate(truthBackward.backwardChi2.begin(), truthBackward.backwardChi2.end(), 0.);
        m_truthSmoothedChi2 = truthRTS.smoothedTotalChi2;
        m_truthSmoothedStatus = truthRTS.smoothedChi2Status;
        m_truthSmoothedError = truthRTS.smoothedChi2Error;
        m_truthForwardLocal = truthRTS.localChi2;
        m_truthBackwardLocal = truthBackward.backwardChi2;
        m_truthSmoothedLocal = truthRTS.smoothedChi2;
        m_truthSmoothedMeasurement = truthRTS.smoothedMeasurementChi2;
        m_truthSmoothedProcess = truthRTS.smoothedProcessChi2;
        m_truthSmoothedNative = truthRTS.smoothedNativeMeasurementChi2;
        m_truthSmoothedSeed = truthRTS.smoothedSeedChi2;
        for (const auto& loss : truthRTS.breakpoints) {
          m_truthRTSLoss.push_back(loss.fittedLogLoss);
          m_truthRTSLossVariance.push_back(loss.fittedVariance);
        }
        for (const auto& loss : truthBackward.breakpoints) {
          m_truthBackwardLoss.push_back(loss.fittedLogLoss);
          m_truthBackwardLossVariance.push_back(loss.fittedVariance);
        }
        auto saveIP = [](const edm4hep::TrackState& ip, std::vector<double>& parameters,
                         std::vector<double>& covariance) {
          parameters = {ip.D0, ip.phi, ip.omega, ip.Z0, ip.tanLambda};
          covariance.assign(ip.covMatrix.begin(), ip.covMatrix.end());
        };
        saveIP(truthRTS.ip, m_truthRTSParameters, m_truthRTSCovariance);
        saveIP(truthBackward.ip, m_truthBackwardParameters, m_truthBackwardCovariance);
        truthRTSIndex = publishTrack(*truthRTSOutput, truthRTS, hits, m_bz,
                                     m_truthSmoothedChi2, adapter, m_caloStates[TruthRTS]);
        truthBackwardIndex = publishTrack(*truthBackwardOutput, truthBackward, hits, m_bz,
                                          m_truthBackwardChi2, adapter,
                                          m_caloStates[TruthBackward]);
        m_truthResultCode = needTruthLoss ? 2 : 1;
        if (m_verbose) {
          for (std::size_t i = 0; i < hits.size(); ++i) {
            for (const auto& named : std::vector<std::pair<const char*, const breakpoint::TrackState*>>{
                {"truth_rts", &truthRTS.endpoint[i]}, {"truth_backward", &truthBackward.endpoint[i]}}) {
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
      } catch (const std::exception& exception) {
        m_truthLikelihood.invalidate(exception.what());
        m_truthResultCode = m_truthOverrideStatus < 0 ? m_truthOverrideStatus : -4;
        truthRTSIndex = truthBackwardIndex = -1;
        m_truthRTSPt = m_truthBackwardPt = m_truthForwardChi2 = m_truthBackwardChi2 = m_truthSmoothedChi2 = nan;
        m_truthSmoothedStatus = 0; m_truthSmoothedError.clear();
        m_truthForwardLocal.clear(); m_truthBackwardLocal.clear(); m_truthSmoothedLocal.clear();
        m_truthSmoothedMeasurement.clear(); m_truthSmoothedProcess.clear(); m_truthSmoothedNative.clear();
        m_truthSmoothedSeed = nan;
        m_truthRTSParameters.clear(); m_truthRTSCovariance.clear();
        m_truthBackwardParameters.clear(); m_truthBackwardCovariance.clear();
        m_truthRTSLoss.clear(); m_truthRTSLossVariance.clear();
        m_truthBackwardLoss.clear(); m_truthBackwardLossVariance.clear();
        m_truthOverrideError = exception.what();
        warning() << "Ordinary pair retained; additional truth pair failed: " << exception.what() << endmsg;
      }
    } catch (const std::exception& exception) {
      warning() << "event=" << m_event << " track=" << m_trackIndex << ": " << exception.what() << endmsg;
      if (m_fitStatus != 1) {
        m_ordinaryLikelihood.reset();
        m_freeLikelihood.reset();
        m_beamFreeLikelihood.reset();
        m_truthLikelihood.reset();
      }
      outputIndices->push_back(-1);
      backwardOutputIndices->push_back(-1);
    }
    statuses->push_back(m_fitStatus);
    truthResultStatuses->push_back(m_truthResultCode);
    truthRTSIndices->push_back(truthRTSIndex);
    truthBackwardIndices->push_back(truthBackwardIndex);
    freeResultStatuses->push_back(m_freeLossTracks.status());
    freeRTSIndices->push_back(m_freeLossTracks.rtsIndex());
    freeBackwardIndices->push_back(m_freeLossTracks.backwardIndex());
    beamFreeResultStatuses->push_back(m_beamFreeLossTracks.status());
    beamFreeRTSIndices->push_back(m_beamFreeLossTracks.rtsIndex());
    beamFreeBackwardIndices->push_back(m_beamFreeLossTracks.backwardIndex());
    diffuseStatuses->push_back(m_diffuseStatus);
    diffuseIndices->push_back(m_diffuseIndex);
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
