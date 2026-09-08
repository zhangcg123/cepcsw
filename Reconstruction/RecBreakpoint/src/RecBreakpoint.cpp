#include "RecBreakpoint.h"
#include "BreakpointFitter.h"
#include "BreakpointTrackSystem.h"
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

DECLARE_COMPONENT(RecBreakpoint)

RecBreakpoint::RecBreakpoint(const std::string& name, ISvcLocator* locator)
    : Algorithm(name, locator) {
  declareProperty("InputTracks", m_input, "Tracks whose reconstructed hits are refitted");
  declareProperty("OutputTracks", m_output, "Successful refitted tracks");
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
  m_backwardModeName = m_backwardMode.value();
  m_lossStateModeName = m_lossStateMode.value();
  if ((m_lossStateModeName != "Persistent6D" && m_lossStateModeName != "LocalMarginal") ||
      (m_lossStateModeName == "Persistent6D" &&
       (m_backwardModeName != "RTS" || m_intervals.value().size() > 1))) {
    error() << "LossStateMode must be LocalMarginal or Persistent6D; Persistent6D requires"
            << " BackwardMode=RTS and at most one breakpoint" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_backwardModeName != "RTS" && m_backwardModeName != "BackwardFilter") {
    error() << "BackwardMode must be RTS or BackwardFilter" << endmsg;
    return StatusCode::FAILURE;
  }
  std::set<int> unique;
  for (int interval : m_intervals.value()) {
    if (interval < 0 || !unique.insert(interval).second) {
      error() << "BreakpointIntervals requires distinct nonnegative indices" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  for (double value : {m_sigmaLoss.value(), m_seedScale.value(), m_maxChi2.value()})
    if (!std::isfinite(value) || value <= 0) {
      error() << "SigmaLogLoss, SeedScale, MaxChi2PerHit must be finite and positive" << endmsg;
      return StatusCode::FAILURE;
    }
  if (!std::isfinite(m_meanLoss.value()) || m_meanLoss < 0 || m_meanLoss > 5) {
    error() << "MeanLogLoss must be finite in [0,5]" << endmsg;
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
  m_tree->Branch("backward_mode", &m_backwardModeName);
  m_tree->Branch("loss_state_mode", &m_lossStateModeName);
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
  info() << "Outward KF + " << m_backwardModeName << "; " << m_intervals.value().size()
         << " selected breakpoint intervals; seed=" << m_seedSelectionName
         << "; Bz=" << m_bz << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecBreakpoint::execute() {
  ++m_event;
  auto* output = m_output.createAndPut();
  auto* statuses = m_status.createAndPut();
  auto* outputIndices = m_outputIndex.createAndPut();
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
  breakpoint::FitSettings settings{m_intervals.value(), m_meanLoss, m_sigmaLoss, m_seedScale};
  settings.backwardMode = m_backwardModeName;
  settings.lossStateMode = m_lossStateModeName;
  m_trackIndex = -1;
  for (const auto& track : *tracks) {
    ++m_trackIndex;
    if (!selected) { statuses->push_back(0); outputIndices->push_back(-1); continue; }
    m_fitStatus = -1;
    m_seedHitIndices.clear();
    m_persistentHits.clear();
    m_sixPredictedMean.clear(); m_sixPredictedCov.clear();
    m_sixFilteredMean.clear(); m_sixFilteredCov.clear();
    m_sixSmoothedMean.clear(); m_sixSmoothedCov.clear();
    m_sixTransport.clear(); m_sixNoise.clear();
    m_kfPt = m_fitPt = m_fitChi2 = m_referencePt = nan;
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
      const auto seedIndices = adapter.seedHitIndices(hits);
      m_seedHitIndices.assign(seedIndices.begin(), seedIndices.end());
      if (m_verbose) info() << "event=" << m_event << " track=" << m_trackIndex
          << " seed=" << m_seedSelectionName << " ordered-hit indices="
          << seedIndices[0] << ',' << seedIndices[1] << ',' << seedIndices[2] << endmsg;
      const auto fit = fitter.fit(hits, settings);
      if (!std::isfinite(fit.ip.omega) || fit.ip.omega == 0)
        throw std::runtime_error("Invalid IP curvature");
      m_fitPt = std::abs(m_bz * 2.99792458e-4 / fit.ip.omega);
      m_fitChi2 = fit.chi2;
      if (m_verifyReference) {
        const auto reference = adapter.referenceKF(hits, m_seedScale,
                                                  m_backwardModeName == "BackwardFilter");
        m_referencePt = std::abs(m_bz * 2.99792458e-4 / reference.omega);
        if (m_intervals.value().empty() &&
            std::abs(m_fitPt / m_referencePt - 1) > 1.e-4)
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
      m_backwardChi2 = fit.backwardChi2;
      for (std::size_t i = 0; i < hits.size(); ++i) {
        m_filteredKappa.push_back(fit.filtered[i].mean(2, 0));
        m_filteredVariance.push_back(fit.filtered[i].covariance(2, 2));
        if (!fit.smoothed.empty()) {
          m_smoothedKappa.push_back(fit.smoothed[i].mean(2, 0));
          m_smoothedVariance.push_back(fit.smoothed[i].covariance(2, 2));
        }
        if (!fit.backwardFiltered.empty()) {
          m_backwardKappa.push_back(fit.backwardFiltered[i].mean(2, 0));
          m_backwardVariance.push_back(fit.backwardFiltered[i].covariance(2, 2));
          m_backwardPredictedKappa.push_back(fit.backwardPredicted[i].mean(2, 0));
          m_backwardPredictedVariance.push_back(fit.backwardPredicted[i].covariance(2, 2));
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
          if (!fit.backwardFiltered.empty()) {
            states.push_back({"backward_predicted", &fit.backwardPredicted[i]});
            states.push_back({"backward_filtered", &fit.backwardFiltered[i]});
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
      if (m_backwardModeName == "RTS") {
        m_rtsLoss = m_fittedLoss;
        m_rtsLossVariance = m_lossVariance;
      }
      outputIndices->push_back(output->size());
      auto result = output->create();
      result.addToTrackStates(fit.ip);
      result.addToTrackStates(breakpoint::toEDM(fit.endpoint.front(), m_bz, 2));
      result.addToTrackStates(breakpoint::toEDM(fit.endpoint.back(), m_bz, 3));
      result.setChi2(fit.chi2);
      // This is the KF innovation degree count, not a calibrated fit test with
      // fitted losses and priors. Persisted chi2 is explicitly filter_chi2.
      result.setNdf(fit.measurementDimensions - 5);
      for (auto hit : hits) result.addToTrackerHits(hit);
      m_fitStatus = 1;
    } catch (const std::exception& exception) {
      warning() << "event=" << m_event << " track=" << m_trackIndex << ": " << exception.what() << endmsg;
      outputIndices->push_back(-1);
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
