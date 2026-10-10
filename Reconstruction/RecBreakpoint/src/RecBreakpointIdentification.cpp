// Diagnostic dataset producer only. This algorithm never invokes a breakpoint
// fitter and never changes CompleteTracks or any reconstruction collection.
#include "BaselineKFDiagnostics.h"
#include "IdentificationMaterial.h"
#include "BreakpointTrackSystem.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Detector.h"
#include "DetInterface/IGeomSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "GearSvc/IGearSvc.h"
#include "TFile.h"
#include "TNamed.h"
#include "TTree.h"
#include "TruthBHLossEventData.h"
#include "UTIL/BitField64.h"
#include "UTIL/ILDConf.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/TrackCollection.h"
#include "k4FWCore/DataHandle.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>

namespace {
using Key = std::pair<int, int>;
template <class T> Key key(const T &x) {
  return {x.id().collectionID, x.id().index};
}
constexpr double missing = std::numeric_limits<double>::quiet_NaN();
template <class T> std::vector<double> xyz(const T &p) {
  return {double(p.x), double(p.y), double(p.z)};
}
void bookDirection(TTree &t, breakpoint::BaselineDirection &d,
                   const std::string &p) {
#define B(name, member) t.Branch((p + name).c_str(), &d.member)
  B("status", status);
  B("error", error);
  B("chi2", chi2);
  B("smoothing_status", smoothingStatus);
  B("seed_parameters", seedParameters);
  B("seed_covariance", seedCovariance);
  B("seed_pivot", seedPivot);
  B("ip_parameters", ipParameters);
  B("ip_covariance", ipCovariance);
  B("production_continuation_status", productionContinuationStatus);
  B("production_ip_parameters", productionIPParameters);
  B("production_ip_covariance", productionIPCovariance);
  B("update_status", updateStatus);
  B("diagnostic_valid", diagnosticValid);
  B("measurement_dimension", dimension);
  B("native_dimension", nativeDimension);
  B("native_predicted_parameters", nativePredicted);
  B("native_predicted_covariance", nativePredictedCovariance);
  B("native_projector", nativeProjector);
  B("native_transport", nativeTransport);
  B("native_process_noise", nativeNoise);
  B("backend_reported_innovation", backendResidual);
  B("previous_accepted_hit", previousHit);
  B("local_chi2", localChi2);
  B("innovation_chi2", residualChi2);
  B("predicted_parameters", predicted);
  B("predicted_covariance", predictedCovariance);
  B("updated_parameters", updated);
  B("updated_covariance", updatedCovariance);
  B("reference_point", pivot);
  B("innovation", innovation);
  B("innovation_covariance", innovationCovariance);
  B("projector", projector);
  B("measurement", measurement);
  B("measurement_covariance", measurementCovariance);
  B("transport", transport);
  B("process_noise", processNoise);
  B("smoothed_parameters", smoothed);
  B("smoothed_covariance", smoothedCovariance);
  B("smoothed_status", smoothStatus);
  B("smoothed_residual", smoothedResidual);
  B("smoothed_residual_covariance", smoothedResidualCovariance);
  B("smoothed_residual_chi2", smoothedResidualChi2);
#undef B
}
} // namespace

class RecBreakpointIdentification final : public Algorithm {
public:
  RecBreakpointIdentification(const std::string &name, ISvcLocator *svc)
      : Algorithm(name, svc) {
    declareProperty("InputTracks", tracks);
    declareProperty("EventHeaders", headers);
    declareProperty("TruthSteps", steps);
    declareProperty("TruthLinks", links);
    declareProperty("TrackTruthAssociations", trackLinks);
    declareProperty("VXDHitAssociations", vxd);
    declareProperty("ITKBarrelHitAssociations", itkb);
    declareProperty("ITKEndcapHitAssociations", itke);
    declareProperty("TPCHitAssociations", tpc);
    declareProperty("OTKBarrelHitAssociations", otkb);
    declareProperty("OTKEndcapHitAssociations", otke);
  }
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  using Association = edm4hep::MCRecoTrackerAssociationCollection;
  DataHandle<edm4hep::TrackCollection> tracks{"CompleteTracks",
                                              Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::EventHeaderCollection> headers{
      "EventHeader", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::G4MaterialStepCollection> steps{
      "GsfG4MaterialSteps", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::SimTrackerHitG4StepLinkCollection> links{
      "GsfSimTrackerHitG4StepLinks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> trackLinks{
      "CompleteTracksParticleAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<Association> vxd{"VXDTrackerHitAssociation",
                              Gaudi::DataHandle::Reader, this};
  DataHandle<Association> itkb{"ITKBarrelTrackerHitAssociation",
                               Gaudi::DataHandle::Reader, this};
  DataHandle<Association> itke{"ITKEndcapTrackerHitAssociation",
                               Gaudi::DataHandle::Reader, this};
  DataHandle<Association> tpc{"TPCTrackerHitAss", Gaudi::DataHandle::Reader,
                              this};
  DataHandle<Association> otkb{"OTKBarrelTrackerHitAssociation",
                               Gaudi::DataHandle::Reader, this};
  DataHandle<Association> otke{"OTKEndcapTrackerHitAssociation",
                               Gaudi::DataHandle::Reader, this};
  Gaudi::Property<std::string> output{this, "TupleFile",
                                      "breakpoint_identification.root"};
  Gaudi::Property<std::string> source{this, "SourceFile", ""};
  Gaudi::Property<std::string> geometryTag{this, "GeometryTag", ""};
  Gaudi::Property<bool> truthEnabled{this, "RecordTruth", true};
  Gaudi::Property<bool> readHeader{this, "ReadEventHeader", false};
  Gaudi::Property<bool> ms{this, "MultipleScattering", true};
  Gaudi::Property<bool> eloss{this, "EnergyLoss", true};
  Gaudi::Property<double> cut{this, "MaxChi2PerHit", 100.};
  Gaudi::Property<double> endpointTolerance{this, "TruthEndpointToleranceMM",
                                            5.};
  Gaudi::Property<std::vector<double>> seedVariances{
      this, "SeedVariances", {1e6, 1e2, 1e-4, 1e6, 1e2}};
  Gaudi::Property<std::vector<int>> selected{this, "SelectedEventIndices", {}};
  Gaudi::Property<bool> verbose{this, "VerboseDiagnostics", false};
  std::unique_ptr<breakpoint::BreakpointTrackSystem> system;
  std::unique_ptr<dd4hep::rec::MaterialManager> material;
  std::unique_ptr<TFile> file;
  TTree *tree = nullptr;
  double bz = 0, baselinePt = missing, baselineChi2 = missing,
         truthPurity = missing, truthLinkedFraction = missing,
         truthWeight = missing;
  int event = -1, eventNumber = -1, runNumber = -1, trackIndex = -1,
      hitCount = 0, baselineNdf = 0, truthStatus = 0, truthTrack = -1,
      truthMC = -1;
  std::string truthError;
  breakpoint::BaselineDirection forward, backward;
  // Maps keep stable addresses from booking until finalize; clear values only.
  std::map<std::string, std::vector<int>> iv;
  std::map<std::string, std::vector<double>> dv;
  std::map<std::string, std::vector<std::vector<double>>> mv;
  std::vector<unsigned long long> cells;
  void recordTruth(const std::vector<edm4hep::TrackerHit> &hits,
                   const edm4hep::Track &track);
};
DECLARE_COMPONENT(RecBreakpointIdentification)

StatusCode RecBreakpointIdentification::initialize() {
  auto sc = Algorithm::initialize();
  if (sc.isFailure())
    return sc;
  if (seedVariances.value().size() != 5 || !std::isfinite(cut.value()) ||
      cut <= 0 || endpointTolerance <= 0)
    return StatusCode::FAILURE;
  for (double v : seedVariances.value())
    if (!std::isfinite(v) || v <= 0)
      return StatusCode::FAILURE;
  auto geom = service<IGeomSvc>("GeomSvc");
  auto gear = service<IGearSvc>("GearSvc");
  if (!geom || !gear || !gear->getGearMgr())
    return StatusCode::FAILURE;
  bz = geom->lcdd()->field().magneticField(dd4hep::Position(0, 0, 0)).z() /
       dd4hep::tesla;
  if (!std::isfinite(bz) || bz == 0)
    return StatusCode::FAILURE;
  system =
      std::make_unique<breakpoint::BreakpointTrackSystem>(*gear->getGearMgr());
  system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useQMS, ms);
  system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::usedEdx, eloss);
  system->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing, false);
  system->init();
  material = std::make_unique<dd4hep::rec::MaterialManager>(
      geom->lcdd()->world().volume());
  const std::filesystem::path path(output.value());
  if (path.has_parent_path())
    std::filesystem::create_directories(path.parent_path());
  file.reset(TFile::Open(path.c_str(), "CREATE"));
  if (!file || file->IsZombie())
    return StatusCode::FAILURE;
  tree = new TTree("interval_identification",
                   "Baseline KF features; truth is labels only");
#define B(name, value) tree->Branch(name, &value)
  B("event_index", event);
  B("event_number", eventNumber);
  B("run_number", runNumber);
  B("input_track_index", trackIndex);
  B("hit_count", hitCount);
  B("baseline_pt", baselinePt);
  B("baseline_chi2", baselineChi2);
  B("baseline_ndf", baselineNdf);
  B("truth_status", truthStatus);
  B("truth_error", truthError);
  B("truth_g4_track_id", truthTrack);
  B("truth_mc_index", truthMC);
  B("truth_match_purity", truthPurity);
  B("truth_linked_hit_fraction", truthLinkedFraction);
  B("truth_association_weight", truthWeight);
  B("hit_cell_id", cells);
#undef B
  for (const auto *n : {"hit_collection_id",
                        "hit_object_index",
                        "hit_original_order",
                        "hit_system",
                        "hit_layer",
                        "hit_side",
                        "interval_upstream_hit",
                        "interval_downstream_hit",
                        "interval_chord_material_status",
                        "interval_chord_material_reverse_status",
                        "interval_chord_material_segments",
                        "interval_chord_material_reverse_segments",
                        "truth_interval_status",
                        "truth_interval_first_step",
                        "truth_interval_last_step",
                        "truth_interval_ebrem_count",
                        "truth_interval_boundary_ambiguity",
                        "truth_hit_link_count",
                        "truth_hit_sim_index",
                        "truth_hit_sim_collection",
                        "truth_hit_g4_track",
                        "truth_hit_hook_step",
                        "truth_hit_hook_status",
                        "truth_ebrem_step",
                        "truth_ebrem_interval",
                        "truth_ebrem_assignment_status"})
    tree->Branch(n, &iv[n]);
  for (const auto *n :
       {"interval_chord_length_mm", "interval_chord_tx0",
        "interval_chord_reverse_tx0", "interval_chord_material_covered_mm",
        "interval_chord_material_reverse_covered_mm", "truth_interval_tx0",
        "truth_interval_ebrem_momentum_loss", "truth_interval_momentum_before",
        "truth_interval_retained_fraction", "truth_interval_start_fraction",
        "truth_interval_end_fraction", "truth_hit_hook_fraction",
        "truth_hit_association_weight", "truth_ebrem_pre_energy",
        "truth_ebrem_post_energy", "truth_ebrem_momentum_loss"})
    tree->Branch(n, &dv[n]);
  for (const auto *n :
       {"hit_position", "hit_position_covariance", "baseline_ip_parameters",
        "baseline_ip_covariance", "truth_particle_momentum",
        "truth_particle_vertex", "truth_ebrem_pre_position",
        "truth_ebrem_post_position", "truth_ebrem_pre_momentum",
        "truth_ebrem_post_momentum"})
    tree->Branch(n, &mv[n]);
  bookDirection(*tree, forward, "forward_");
  bookDirection(*tree, backward, "backward_");
  std::ostringstream metadata;
  metadata << "schema=2; source=" << source.value()
           << "; geometry=" << geometryTag.value()
           << "; units=mm,GeV,rad; state=(drho,phi0,kappa=q/pT,dz,tanLambda); "
              "matrices=row-major; "
           << "hit order=stable increasing cylindrical radius (barrel first "
              "version); smoothing=native forward KalTest; "
           << "seed=standard first/middle/last 2D hits, includes both track "
              "ends; independent loose covariance per direction; "
           << "updated/smoothed native states roundtrip through float EDM; "
              "prediction matrices native double; "
           << "no breakpoint, no ECAL, no truth steering; rejected matrix "
              "slots empty; "
           << "interval_chord_length is NOT curved path length; runtime "
              "material represented by per-transition Q; interval_chord_tx0 "
              "is a separate DD4hep reconstructed-hit chord scan, not KF path; "
           << "smoothed residual covariance=V-H*P_s*H^T for same affine "
              "forward model; "
           << "truth interval loss is momentum loss, not photon energy; truth "
              "assignment uses exact provenance hooks; "
           << "Bz=" << bz << "; MS=" << ms.value() << "; dEdx=" << eloss.value()
           << "; cut=" << cut.value() << "; seed variances=";
  for (double v : seedVariances.value())
    metadata << v << ',';
  TNamed("schema_and_configuration", metadata.str().c_str()).Write();
  return StatusCode::SUCCESS;
}

StatusCode RecBreakpointIdentification::execute() {
  ++event;
  if (!selected.value().empty() &&
      std::find(selected.value().begin(), selected.value().end(), event) ==
          selected.value().end())
    return StatusCode::SUCCESS;
  eventNumber = runNumber = -1;
  if (readHeader) {
    try {
      if (const auto *collection = headers.get())
        for (const auto &h : *collection) {
          eventNumber = h.getEventNumber();
          runNumber = h.getRunNumber();
          break;
        }
    } catch (...) {
    }
  }
  trackIndex = -1;
  for (const auto &track : *tracks.get()) {
    ++trackIndex;
    for (auto &v : iv)
      v.second.clear();
    for (auto &v : dv)
      v.second.clear();
    for (auto &v : mv)
      v.second.clear();
    cells.clear();
    baselinePt = missing;
    baselineChi2 = track.getChi2();
    baselineNdf = track.getNdf();
    for (const auto &state : track.getTrackStates())
      if (state.location == edm4hep::TrackState::AtIP) {
        const auto s = breakpoint::fromEDM(state, bz);
        mv["baseline_ip_parameters"].push_back({s.mean(0, 0), s.mean(1, 0),
                                                s.mean(2, 0), s.mean(3, 0),
                                                s.mean(4, 0)});
        std::vector<double> cov;
        for (int i = 0; i < 5; ++i)
          for (int j = 0; j < 5; ++j)
            cov.push_back(s.covariance(i, j));
        mv["baseline_ip_covariance"].push_back(cov);
        if (state.omega != 0)
          baselinePt = std::abs(bz * 2.99792458e-4 / state.omega);
        break;
      }
    std::vector<edm4hep::TrackerHit> hits;
    std::map<Key, int> original;
    for (auto h : track.getTrackerHits()) {
      original[key(h)] = hits.size();
      hits.push_back(h);
    }
    std::stable_sort(hits.begin(), hits.end(), [](auto a, auto b) {
      const auto p = a.getPosition(), q = b.getPosition();
      return std::hypot(p.x, p.y) < std::hypot(q.x, q.y);
    });
    hitCount = hits.size();
    UTIL::BitField64 decoder(UTIL::ILDCellID0::encoder_string);
    for (int i = 0; i < hitCount; ++i) {
      const auto h = hits[i];
      const auto p = h.getPosition();
      iv["hit_collection_id"].push_back(key(h).first);
      iv["hit_object_index"].push_back(key(h).second);
      iv["hit_original_order"].push_back(original.at(key(h)));
      cells.push_back(h.getCellID());
      decoder.setValue(h.getCellID());
      iv["hit_system"].push_back(decoder["subdet"]);
      iv["hit_layer"].push_back(decoder["layer"]);
      iv["hit_side"].push_back(decoder["side"]);
      mv["hit_position"].push_back(xyz(p));
      const auto c = h.getCovMatrix();
      mv["hit_position_covariance"].push_back(
          std::vector<double>(c.begin(), c.end()));
      if (i) {
        const auto q = hits[i - 1].getPosition();
        iv["interval_upstream_hit"].push_back(i - 1);
        iv["interval_downstream_hit"].push_back(i);
        dv["interval_chord_length_mm"].push_back(
            std::sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y) +
                      (p.z - q.z) * (p.z - q.z)));
        const TVector3 from(q.x, q.y, q.z), to(p.x, p.y, p.z);
        const auto outward = breakpoint::identificationMaterial(*material, from, to);
        const auto inward = breakpoint::identificationMaterial(*material, to, from);
        dv["interval_chord_tx0"].push_back(outward.tx0);
        dv["interval_chord_reverse_tx0"].push_back(inward.tx0);
        dv["interval_chord_material_covered_mm"].push_back(outward.coveredMM);
        dv["interval_chord_material_reverse_covered_mm"].push_back(inward.coveredMM);
        iv["interval_chord_material_status"].push_back(outward.status);
        iv["interval_chord_material_reverse_status"].push_back(inward.status);
        iv["interval_chord_material_segments"].push_back(outward.segments);
        iv["interval_chord_material_reverse_segments"].push_back(inward.segments);
      }
    }
    // Reconstructed features are finalized BEFORE the first truth lookup.
    forward = breakpoint::baselineDiagnostics(
        *system, hits, bz, seedVariances.value(), cut, false, true);
    backward = breakpoint::baselineDiagnostics(
        *system, hits, bz, seedVariances.value(), cut, true, false);
    truthStatus = 0;
    truthError.clear();
    truthTrack = truthMC = -1;
    truthPurity = truthLinkedFraction = truthWeight = missing;
    if (truthEnabled)
      try {
        recordTruth(hits, track);
      } catch (const std::exception &e) {
        truthStatus = -1;
        truthError = e.what();
      }
    if (verbose) {
      info() << "event=" << event << " track=" << trackIndex
             << " hits=" << hitCount << " F=" << forward.status
             << " B=" << backward.status << " truth=" << truthStatus << endmsg;
      for (int i = 0; i < hitCount; ++i) {
        std::ostringstream s;
        s.precision(17);
        s << "hit=" << i << " Fchi2=" << forward.localChi2[i]
          << " Bchi2=" << backward.localChi2[i]
          << " Sstatus=" << forward.smoothStatus[i] << " Fpred=[";
        for (double x : forward.predicted[i])
          s << x << ',';
        s << "] Fcov=[";
        for (double x : forward.predictedCovariance[i])
          s << x << ',';
        s << ']';
        info() << s.str() << endmsg;
      }
    }
    tree->Fill();
  }
  return StatusCode::SUCCESS;
}

void RecBreakpointIdentification::recordTruth(
    const std::vector<edm4hep::TrackerHit> &hits, const edm4hep::Track &track) {
  std::vector<const Association *> associations;
  for (auto *h : {&vxd, &itkb, &itke, &tpc, &otkb, &otke})
    try {
      associations.push_back(h->get());
    } catch (...) {
    }
  std::map<Key, std::vector<edm4hep::MCRecoTrackerAssociation>> byHit;
  for (const auto *a : associations)
    if (a)
      for (const auto &link : *a)
        if (link.getRec().isAvailable() && link.getSim().isAvailable())
          byHit[key(link.getRec())].push_back(link);
  std::map<Key, gsftruth::SimTrackerHitG4StepLink> hooks;
  for (const auto &l : *links.get())
    if (l.getSimHit().isAvailable())
      hooks[key(l.getSimHit())] = l;
  std::map<Key, int> counts;
  int linkedHits = 0;
  for (const auto &hit : hits) {
    const auto &matches = byHit[key(hit)];
    std::set<Key> particles;
    for (const auto &a : matches)
      if (a.getSim().getMCParticle().isAvailable())
        particles.insert(key(a.getSim().getMCParticle()));
    if (!particles.empty())
      ++linkedHits;
    for (const auto &k : particles)
      ++counts[k];
    iv["truth_hit_link_count"].push_back(matches.size());
    // Do not silently pick one of multiple simulated hits.
    const bool unique = matches.size() == 1;
    const auto it =
        unique ? hooks.find(key(matches.front().getSim())) : hooks.end();
    iv["truth_hit_sim_index"].push_back(
        unique ? key(matches.front().getSim()).second : -1);
    iv["truth_hit_sim_collection"].push_back(
        unique ? key(matches.front().getSim()).first : -1);
    dv["truth_hit_association_weight"].push_back(
        unique ? matches.front().getWeight() : missing);
    iv["truth_hit_g4_track"].push_back(
        it != hooks.end() ? it->second.getTrackID() : -1);
    iv["truth_hit_hook_step"].push_back(
        it != hooks.end() ? it->second.getHookStepNumber() : -1);
    iv["truth_hit_hook_status"].push_back(
        it != hooks.end() ? it->second.getStatus() : -1);
    dv["truth_hit_hook_fraction"].push_back(
        it != hooks.end() ? it->second.getHookFraction() : missing);
  }
  truthLinkedFraction =
      hits.empty() ? missing : double(linkedHits) / hits.size();
  // Select the largest explicit track association; purity is the fraction of
  // ALL track hits linked to this MC particle (unlinked hits stay denominator).
  edm4hep::MCParticle best;
  int ties = 0;
  for (const auto &a : *trackLinks.get())
    if (a.getRec() == track && a.getSim().isAvailable()) {
      if (!std::isfinite(truthWeight) || a.getWeight() > truthWeight) {
        truthWeight = a.getWeight();
        best = a.getSim();
        ties = 1;
      } else if (a.getWeight() == truthWeight)
        ++ties;
    }
  if (best.isAvailable() && ties == 1) {
    truthMC = best.id().index;
    truthPurity =
        hits.empty() ? missing : double(counts[key(best)]) / hits.size();
    mv["truth_particle_momentum"].push_back(xyz(best.getMomentum()));
    mv["truth_particle_vertex"].push_back(xyz(best.getVertex()));
  }
  TruthBHLossEventData reader;
  TruthBHLossEventDataMatch match;
  const auto *material = steps.get();
  if (!reader.prepare(material, links.get(), associations, truthError) ||
      !reader.matchTrack(hits, endpointTolerance, true, match, truthError)) {
    truthStatus = -1;
    iv["truth_interval_status"].assign(hits.empty() ? 0 : hits.size() - 1, -1);
    return;
  }
  truthTrack = match.g4TrackID;
  truthStatus = ties > 1 ? 2 : 1;
  for (const auto &interval : match.materialIntervals) {
    iv["truth_interval_status"].push_back(1);
    iv["truth_interval_first_step"].push_back(interval.firstStepNumber);
    iv["truth_interval_last_step"].push_back(interval.lastStepNumber);
    dv["truth_interval_start_fraction"].push_back(interval.startHookFraction);
    dv["truth_interval_end_fraction"].push_back(interval.endHookFraction);
    dv["truth_interval_tx0"].push_back(interval.truthTX0);
    dv["truth_interval_ebrem_momentum_loss"].push_back(interval.ebremLoss);
    dv["truth_interval_momentum_before"].push_back(interval.momentumBefore);
    dv["truth_interval_retained_fraction"].push_back(interval.retainedFraction);
    iv["truth_interval_ebrem_count"].push_back(0);
    iv["truth_interval_boundary_ambiguity"].push_back(0);
  }
  for (const auto &s : *material)
    if (s.getTrackID() == truthTrack && s.getProcessSubtype() == 3) {
      const int step = s.getStepNumber();
      int owner = -3, assignment = 0;
      // Emission occurs at the G4 post-step point. Boundary-step emissions are
      // flagged separately; full hooks remain available for downstream policy.
      for (std::size_t i = 0; i < match.materialIntervals.size(); ++i) {
        const auto &v = match.materialIntervals[i];
        if (step >= v.firstStepNumber && step <= v.lastStepNumber) {
          if (step == v.firstStepNumber || step == v.lastStepNumber)
            iv["truth_interval_boundary_ambiguity"][i] = 1;
          if ((step > v.firstStepNumber || v.startHookFraction < 1. - 1.e-6) &&
              (step < v.lastStepNumber || v.endHookFraction >= 1. - 1.e-6)) {
            owner = i;
            assignment =
                (step == v.firstStepNumber || step == v.lastStepNumber) ? 2 : 1;
            ++iv["truth_interval_ebrem_count"][i];
            break;
          }
        }
      }
      if (owner == -3 && !match.materialIntervals.empty()) {
        const auto &a = match.materialIntervals.front();
        const auto &b = match.materialIntervals.back();
        if (step < a.firstStepNumber ||
            (step == a.firstStepNumber && a.startHookFraction >= 1. - 1.e-6)) {
          owner = -1;
          assignment = 3;
        }
        if (step > b.lastStepNumber ||
            (step == b.lastStepNumber && b.endHookFraction < 1. - 1.e-6)) {
          owner = -2;
          assignment = 4;
        }
      }
      iv["truth_ebrem_step"].push_back(step);
      iv["truth_ebrem_interval"].push_back(owner);
      iv["truth_ebrem_assignment_status"].push_back(assignment);
      mv["truth_ebrem_pre_position"].push_back(xyz(s.getPrePosition()));
      mv["truth_ebrem_post_position"].push_back(xyz(s.getPostPosition()));
      mv["truth_ebrem_pre_momentum"].push_back(xyz(s.getPreMomentum()));
      mv["truth_ebrem_post_momentum"].push_back(xyz(s.getPostMomentum()));
      dv["truth_ebrem_pre_energy"].push_back(s.getPreKineticEnergy() +
                                             0.00051099895);
      dv["truth_ebrem_post_energy"].push_back(s.getPostKineticEnergy() +
                                              0.00051099895);
      dv["truth_ebrem_momentum_loss"].push_back(s.getMomentumLoss());
    }
}

StatusCode RecBreakpointIdentification::finalize() {
  if (file) {
    file->cd();
    tree->Write();
    file->Close();
  }
  return Algorithm::finalize();
}
