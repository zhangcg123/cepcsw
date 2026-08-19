#include "GsfMaterialStepRecorderAnaElemTool.h"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4ParticleDefinition.hh"
#include "G4Run.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4VSensitiveDetector.hh"
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/MaterialManager.h"

#include <TFile.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <stdexcept>

DECLARE_COMPONENT(GsfMaterialStepRecorderAnaElemTool)

namespace {
  float mag3(const G4ThreeVector& v) {
    return (float)(v.mag() / CLHEP::GeV);
  }

  float pt(const G4ThreeVector& v) {
    return (float)(std::hypot(v.x(), v.y()) / CLHEP::GeV);
  }

  float toMm(double v) {
    return (float)(v / CLHEP::mm);
  }

  float gev(double v) {
    return (float)(v / CLHEP::GeV);
  }

  std::string touchablePath(const G4StepPoint* point) {
    if (!point) return "";
    const auto& touchable = point->GetTouchableHandle();
    if (!touchable) return "";

    std::ostringstream path;
    const int depth = touchable->GetHistoryDepth();
    for (int level = depth; level >= 0; --level) {
      const auto* volume = touchable->GetVolume(level);
      if (!volume) continue;
      path << '/' << volume->GetName() << '[' << volume->GetCopyNo() << ']';
    }
    return path.str();
  }

  int isSensitive(const G4VPhysicalVolume* volume) {
    return volume && volume->GetLogicalVolume() &&
           volume->GetLogicalVolume()->GetSensitiveDetector() ? 1 : 0;
  }

  bool isTpcUpper(const std::string& path) {
    return path.find("/TPC_upperlayer_log_") != std::string::npos;
  }

  bool isTpcLower(const std::string& path) {
    return path.find("/TPC_lowerlayer_log_") != std::string::npos;
  }
}

StatusCode GsfMaterialStepRecorderAnaElemTool::initialize() {
  if (m_recordDD4hepSurfaceIntervals.value() &&
      (!m_recordZeroLoss.value() || m_minStepLengthMm.value() != 0.0 ||
       m_minAbsLossGeV.value() != 0.0)) {
    error() << "RecordDD4hepSurfaceIntervals requires RecordZeroLoss=true, "
            << "MinStepLengthMm=0, and MinAbsLossGeV=0 so interval material "
            << "is complete" << endmsg;
    return StatusCode::FAILURE;
  }
  m_file = TFile::Open(m_outputFile.value().c_str(), "RECREATE");
  if (!m_file || m_file->IsZombie()) {
    error() << "Cannot create output file " << m_outputFile << endmsg;
    return StatusCode::FAILURE;
  }
  bookTree();
  return StatusCode::SUCCESS;
}

StatusCode GsfMaterialStepRecorderAnaElemTool::finalize() {
  if (m_file) {
    m_file->cd();
    if (m_tree) m_tree->Write();
    if (m_dd4hepTree) m_dd4hepTree->Write();
    m_file->Close();
    delete m_file;
    m_file = nullptr;
  }
  delete m_materialManager;
  m_materialManager = nullptr;
  return StatusCode::SUCCESS;
}

void GsfMaterialStepRecorderAnaElemTool::BeginOfRunAction(const G4Run* run) {
  m_run_id = run ? run->GetRunID() : -1;
  m_geosvc = service<IGeomSvc>("GeomSvc");
  if (!m_geosvc) {
    warning() << "GeomSvc unavailable; TrackerOnly will not apply geometry bounds" << endmsg;
    return;
  }

  auto* dd4hepGeo = m_geosvc->lcdd();
  if (m_recordDD4hepSurfaceIntervals.value() && !m_materialManager) {
    m_materialManager = new dd4hep::rec::MaterialManager(
        dd4hepGeo->world().volume());
  }
  try {
    // DD4hep constants use DD4hep's internal length unit (centimetres), while
    // Geant4 positions below are explicitly converted to millimetres.
    m_trackerR = dd4hepGeo->constant<double>("tracker_region_rmax") / dd4hep::mm;
    m_trackerZ = dd4hepGeo->constant<double>("tracker_region_zmax") / dd4hep::mm;
    info() << "GsfMaterialStepRecorder tracker region R=" << m_trackerR
           << " mm Z=" << m_trackerZ << " mm" << endmsg;
  } catch (std::runtime_error&) {
    m_trackerR = 0.0;
    m_trackerZ = 0.0;
    warning() << "tracker_region_rmax/zmax constants unavailable; TrackerOnly disabled" << endmsg;
  }
}

void GsfMaterialStepRecorderAnaElemTool::BeginOfEventAction(const G4Event* event) {
  m_event_id = event ? event->GetEventID() : -1;
  clearVectors();
}

void GsfMaterialStepRecorderAnaElemTool::EndOfEventAction(const G4Event*) {
  m_step_count = (int)m_track_id.size();
  m_tree->Fill();
  if (m_recordDD4hepSurfaceIntervals.value()) {
    buildDD4hepSurfaceIntervals();
    m_dd4hep_interval_count = (int)m_dd4hep_track_id.size();
    m_dd4hepTree->Fill();
  }
}

void GsfMaterialStepRecorderAnaElemTool::buildDD4hepSurfaceIntervals() {
  if (!m_materialManager) return;

  struct Sample {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double p = 0.0;
  };
  struct Anchor {
    int trackStep = 0;
    double trackLength = 0.0;
    Sample point;
    std::string surface;
  };

  std::map<int, std::vector<size_t>> tracks;
  for (size_t index = 0; index < m_track_id.size(); ++index)
    tracks[m_track_id[index]].push_back(index);

  for (auto& track : tracks) {
    auto& indices = track.second;
    std::sort(indices.begin(), indices.end(), [&](size_t lhs, size_t rhs) {
      if (m_track_step_number[lhs] != m_track_step_number[rhs])
        return m_track_step_number[lhs] < m_track_step_number[rhs];
      return m_recorded_step_index[lhs] < m_recorded_step_index[rhs];
    });

    auto sampleAt = [&](double trackLength) {
      Sample sample;
      if (indices.empty()) return sample;
      size_t selected = indices.back();
      double fraction = 1.0;
      for (const auto index : indices) {
        const double begin = m_track_length_pre[index];
        const double end = m_track_length_post[index];
        if (trackLength < begin) {
          selected = index;
          fraction = 0.0;
          break;
        }
        if (trackLength <= end || index == indices.back()) {
          selected = index;
          fraction = end > begin
              ? std::clamp((trackLength - begin) / (end - begin), 0.0, 1.0)
              : 1.0;
          break;
        }
      }
      sample.x = m_pre_x[selected] +
          fraction * (m_post_x[selected] - m_pre_x[selected]);
      sample.y = m_pre_y[selected] +
          fraction * (m_post_y[selected] - m_pre_y[selected]);
      sample.z = m_pre_z[selected] +
          fraction * (m_post_z[selected] - m_pre_z[selected]);
      sample.p = m_pre_p[selected] +
          fraction * (m_post_p[selected] - m_pre_p[selected]);
      return sample;
    };

    std::vector<Anchor> anchors;
    for (size_t order = 0; order < indices.size(); ++order) {
      const auto index = indices[order];
      if (!m_pre_sensitive[index] ||
          m_step_status_pre[index] != (int)fGeomBoundary)
        continue;
      const std::string surface = m_pre_touchable_path[index];
      if (isTpcUpper(surface)) continue;

      // Find the exit from this sensitive element.  A CEPC TPC pad row is
      // represented by adjacent lower/upper sensitive half-volumes, so treat
      // the pair as one measurement layer and use the pair midpoint.
      size_t endOrder = order;
      while (endOrder + 1 < indices.size() &&
             m_pre_sensitive[indices[endOrder + 1]] &&
             m_pre_touchable_path[indices[endOrder + 1]] == surface)
        ++endOrder;
      if (isTpcLower(surface) && endOrder + 1 < indices.size()) {
        const std::string upper =
            m_pre_touchable_path[indices[endOrder + 1]];
        if (m_pre_sensitive[indices[endOrder + 1]] && isTpcUpper(upper)) {
          ++endOrder;
          while (endOrder + 1 < indices.size() &&
                 m_pre_sensitive[indices[endOrder + 1]] &&
                 m_pre_touchable_path[indices[endOrder + 1]] == upper)
            ++endOrder;
        }
      }

      const double entryLength = m_track_length_pre[index];
      const double exitLength = m_track_length_post[indices[endOrder]];
      if (!(exitLength > entryLength)) continue;
      const double midpointLength = 0.5 * (entryLength + exitLength);
      anchors.push_back({m_track_step_number[index], midpointLength,
                         sampleAt(midpointLength), surface});
    }

    for (size_t interval = 0; interval + 1 < anchors.size(); ++interval) {
      const auto& from = anchors[interval];
      const auto& to = anchors[interval + 1];
      if (!(to.trackLength > from.trackLength)) continue;

      const dd4hep::rec::Vector3D p0(
          from.point.x * dd4hep::mm, from.point.y * dd4hep::mm,
          from.point.z * dd4hep::mm);
      const dd4hep::rec::Vector3D p1(
          to.point.x * dd4hep::mm, to.point.y * dd4hep::mm,
          to.point.z * dd4hep::mm);
      double dd4hepTX0 = 0.0;
      int dd4hepSegments = 0;
      std::ostringstream dd4hepAudit;
      try {
        const auto& materials = m_materialManager->materialsBetween(p0, p1);
        for (const auto& segment : materials) {
          const double radLength = segment.first.radLength();
          if (!(radLength > 0.0) || !(segment.second > 0.0)) continue;
          const double tx0 = segment.second / radLength;
          dd4hepTX0 += tx0;
          ++dd4hepSegments;
          if (dd4hepAudit.tellp() > 0) dd4hepAudit << '|';
          dd4hepAudit << segment.first.name() << ':'
                      << segment.second / dd4hep::mm << ':' << tx0;
        }
      } catch (const std::exception& exception) {
        warning() << "DD4hep midpoint material query failed for event "
                  << m_event_id << " track " << track.first << " interval "
                  << interval << ": " << exception.what() << endmsg;
      }

      double g4TX0 = 0.0;
      double ebremLoss = 0.0;
      int g4Steps = 0;
      int ebremSteps = 0;
      for (const auto index : indices) {
        const double begin = m_track_length_pre[index];
        const double end = m_track_length_post[index];
        const double overlap =
            std::max(0.0, std::min(end, to.trackLength) -
                              std::max(begin, from.trackLength));
        if (overlap > 0.0 && end > begin) {
          g4TX0 += m_step_tX0[index] * overlap / (end - begin);
          ++g4Steps;
        }
        // A discrete Geant4 process is attached to the post-step point.
        if (m_process_subtype[index] == 3 && end > from.trackLength &&
            end <= to.trackLength) {
          ++ebremSteps;
          ebremLoss += m_loss[index];
        }
      }

      const double dx = to.point.x - from.point.x;
      const double dy = to.point.y - from.point.y;
      const double dz = to.point.z - from.point.z;
      m_dd4hep_track_id.push_back(track.first);
      m_dd4hep_parent_id.push_back(m_parent_id[indices.front()]);
      m_dd4hep_is_primary.push_back(m_is_primary[indices.front()]);
      m_dd4hep_pdg.push_back(m_pdg[indices.front()]);
      m_dd4hep_interval_index.push_back((int)interval);
      m_dd4hep_from_track_step.push_back(from.trackStep);
      m_dd4hep_to_track_step.push_back(to.trackStep);
      m_dd4hep_segment_count.push_back(dd4hepSegments);
      m_dd4hep_valid.push_back(dd4hepSegments > 0 &&
          std::isfinite(dd4hepTX0) && dd4hepTX0 > 0.0 ? 1 : 0);
      m_dd4hep_g4_step_count.push_back(g4Steps);
      m_dd4hep_ebrem_step_count.push_back(ebremSteps);
      m_dd4hep_from_x.push_back(from.point.x);
      m_dd4hep_from_y.push_back(from.point.y);
      m_dd4hep_from_z.push_back(from.point.z);
      m_dd4hep_from_r.push_back(std::hypot(from.point.x, from.point.y));
      m_dd4hep_to_x.push_back(to.point.x);
      m_dd4hep_to_y.push_back(to.point.y);
      m_dd4hep_to_z.push_back(to.point.z);
      m_dd4hep_to_r.push_back(std::hypot(to.point.x, to.point.y));
      m_dd4hep_from_track_length_mm.push_back(from.trackLength);
      m_dd4hep_to_track_length_mm.push_back(to.trackLength);
      m_dd4hep_path_length_mm.push_back(std::sqrt(dx * dx + dy * dy + dz * dz));
      m_dd4hep_path_tX0.push_back(dd4hepTX0);
      m_dd4hep_g4_tX0.push_back(g4TX0);
      m_dd4hep_p_before_GeV.push_back(from.point.p);
      m_dd4hep_p_after_GeV.push_back(to.point.p);
      m_dd4hep_ebrem_loss_GeV.push_back(ebremLoss);
      m_dd4hep_surface_from.push_back(from.surface);
      m_dd4hep_surface_to.push_back(to.surface);
      m_dd4hep_materials.push_back(dd4hepAudit.str());
    }
  }
}

bool GsfMaterialStepRecorderAnaElemTool::acceptPdg(int pdg) const {
  for (const auto allowed : m_pdgs.value()) {
    if (pdg == allowed) return true;
  }
  return false;
}

bool GsfMaterialStepRecorderAnaElemTool::insideTracker(double r, double z) const {
  if (m_trackerR <= 0.0 || m_trackerZ <= 0.0) return true;
  return r < m_trackerR && std::fabs(z) < m_trackerZ;
}

void GsfMaterialStepRecorderAnaElemTool::UserSteppingAction(const G4Step* step) {
  if (!step) return;
  const auto* track = step->GetTrack();
  if (!track) return;
  const auto* particle = track->GetDefinition();
  if (!particle) return;

  const int pdg = particle->GetPDGEncoding();
  if (!acceptPdg(pdg)) return;
  if (m_primaryOnly.value() && track->GetParentID() != 0) return;
  if (step->GetStepLength() / CLHEP::mm < m_minStepLengthMm.value()) return;

  const auto* pre = step->GetPreStepPoint();
  const auto* post = step->GetPostStepPoint();
  if (!pre || !post) return;

  const auto& prePos = pre->GetPosition();
  const auto& postPos = post->GetPosition();
  const double preR = prePos.perp() / CLHEP::mm;
  const double postR = postPos.perp() / CLHEP::mm;
  const double preZ = prePos.z() / CLHEP::mm;
  const double postZ = postPos.z() / CLHEP::mm;
  if (m_trackerOnly.value() && !insideTracker(preR, preZ) && !insideTracker(postR, postZ)) return;

  const auto& preMom = pre->GetMomentum();
  const auto& postMom = post->GetMomentum();
  const double preP = preMom.mag() / CLHEP::GeV;
  const double postP = postMom.mag() / CLHEP::GeV;
  const double loss = preP - postP;
  if (!m_recordZeroLoss.value() && loss <= 0.0) return;
  if (std::fabs(loss) < m_minAbsLossGeV.value()) return;

  // ── push step-level data into vectors ──

  m_track_id.push_back(track->GetTrackID());
  m_parent_id.push_back(track->GetParentID());
  m_is_primary.push_back(track->GetParentID() == 0 ? 1 : 0);
  m_track_step_number.push_back(track->GetCurrentStepNumber());
  m_recorded_step_index.push_back((int)m_recorded_step_index.size());
  m_pdg.push_back(pdg);
  m_charge.push_back((int)std::lround(particle->GetPDGCharge()));
  m_step_status_pre.push_back((int)pre->GetStepStatus());
  m_step_status_post.push_back((int)post->GetStepStatus());

  m_pre_x.push_back(toMm(prePos.x()));
  m_pre_y.push_back(toMm(prePos.y()));
  m_pre_z.push_back(toMm(prePos.z()));
  m_pre_r.push_back((float)preR);
  m_post_x.push_back(toMm(postPos.x()));
  m_post_y.push_back(toMm(postPos.y()));
  m_post_z.push_back(toMm(postPos.z()));
  m_post_r.push_back((float)postR);

  float mx = 0.5f * (m_pre_x.back() + m_post_x.back());
  float my = 0.5f * (m_pre_y.back() + m_post_y.back());
  float mz = 0.5f * (m_pre_z.back() + m_post_z.back());
  m_mid_x.push_back(mx);
  m_mid_y.push_back(my);
  m_mid_z.push_back(mz);
  m_mid_r.push_back(std::hypot(mx, my));

  float px_pre = gev(preMom.x()), py_pre = gev(preMom.y()), pz_pre = gev(preMom.z());
  float px_post = gev(postMom.x()), py_post = gev(postMom.y()), pz_post = gev(postMom.z());
  float pp = mag3(preMom), ppt = pt(preMom);
  float ppost = mag3(postMom), ppt_post = pt(postMom);

  m_pre_px.push_back(px_pre);
  m_pre_py.push_back(py_pre);
  m_pre_pz.push_back(pz_pre);
  m_pre_p.push_back(pp);
  m_pre_pT.push_back(ppt);

  m_post_px.push_back(px_post);
  m_post_py.push_back(py_post);
  m_post_pz.push_back(pz_post);
  m_post_p.push_back(ppost);
  m_post_pT.push_back(ppt_post);

  const auto& preDir = pre->GetMomentumDirection();
  const auto& postDir = post->GetMomentumDirection();
  m_pre_dir_x.push_back((float)preDir.x());
  m_pre_dir_y.push_back((float)preDir.y());
  m_pre_dir_z.push_back((float)preDir.z());
  m_post_dir_x.push_back((float)postDir.x());
  m_post_dir_y.push_back((float)postDir.y());
  m_post_dir_z.push_back((float)postDir.z());

  m_dp.push_back(ppost - pp);
  m_loss.push_back(pp - ppost);
  m_retained.push_back((pp > 0.0f) ? ppost / pp : 0.0f);

  m_pre_ekin.push_back(gev(pre->GetKineticEnergy()));
  m_post_ekin.push_back(gev(post->GetKineticEnergy()));
  m_dekin.push_back(m_pre_ekin.back() - m_post_ekin.back());
  m_edep.push_back(gev(step->GetTotalEnergyDeposit()));
  m_nonion_edep.push_back(gev(step->GetNonIonizingEnergyDeposit()));
  m_step_length.push_back(toMm(step->GetStepLength()));
  const float trackLengthPost = toMm(track->GetTrackLength());
  m_track_length_post.push_back(trackLengthPost);
  m_track_length_pre.push_back(trackLengthPost - m_step_length.back());
  m_global_time_pre.push_back((float)(pre->GetGlobalTime() / CLHEP::ns));
  m_global_time_post.push_back((float)(post->GetGlobalTime() / CLHEP::ns));

  const auto* material = pre->GetMaterial();
  if (material) {
    m_material.push_back(material->GetName());
    float radlen = toMm(material->GetRadlen());
    m_material_radlen.push_back(radlen);
    m_step_tX0.push_back((radlen > 0.0f) ? m_step_length.back() / radlen : 0.0f);
  } else {
    m_material.push_back("");
    m_material_radlen.push_back(0.0f);
    m_step_tX0.push_back(0.0f);
  }

  const auto* preVol = pre->GetPhysicalVolume();
  if (preVol) {
    m_pre_volume.push_back(preVol->GetName());
    m_pre_volume_copy_no.push_back(preVol->GetCopyNo());
  } else {
    m_pre_volume.push_back("");
    m_pre_volume_copy_no.push_back(0);
  }
  m_pre_sensitive.push_back(isSensitive(preVol));
  m_pre_touchable_path.push_back(touchablePath(pre));

  const auto* postVol = post->GetPhysicalVolume();
  if (postVol) {
    m_post_volume.push_back(postVol->GetName());
    m_post_volume_copy_no.push_back(postVol->GetCopyNo());
  } else {
    m_post_volume.push_back("");
    m_post_volume_copy_no.push_back(0);
  }
  m_post_sensitive.push_back(isSensitive(postVol));
  m_post_touchable_path.push_back(touchablePath(post));

  const auto* process = post->GetProcessDefinedStep();
  if (process) {
    m_process.push_back(process->GetProcessName());
    m_process_subtype.push_back(process->GetProcessSubType());
  } else {
    m_process.push_back("");
    m_process_subtype.push_back(0);
  }
}

void GsfMaterialStepRecorderAnaElemTool::bookTree() {
  m_tree = new TTree("g4step_tuple", "GSF Geant4 pre/post-step material-loss tuple (one entry per event)");
  m_tree->SetDirectory(m_file);

  // ── event-level branches ──
  m_tree->Branch("run_id",      &m_run_id);
  m_tree->Branch("event_id",    &m_event_id);
  m_tree->Branch("step_count",  &m_step_count);

  // ── step-level vector branches ──
  m_tree->Branch("track_id",    &m_track_id);
  m_tree->Branch("parent_id",   &m_parent_id);
  m_tree->Branch("is_primary",  &m_is_primary);
  m_tree->Branch("track_step_number", &m_track_step_number);
  m_tree->Branch("recorded_step_index", &m_recorded_step_index);
  m_tree->Branch("pdg",         &m_pdg);
  m_tree->Branch("charge",      &m_charge);
  m_tree->Branch("step_status_pre",   &m_step_status_pre);
  m_tree->Branch("step_status_post",  &m_step_status_post);
  m_tree->Branch("process_subtype",   &m_process_subtype);
  m_tree->Branch("pre_volume_copy_no",  &m_pre_volume_copy_no);
  m_tree->Branch("post_volume_copy_no", &m_post_volume_copy_no);
  m_tree->Branch("pre_sensitive",  &m_pre_sensitive);
  m_tree->Branch("post_sensitive", &m_post_sensitive);

  m_tree->Branch("pre_x",  &m_pre_x);
  m_tree->Branch("pre_y",  &m_pre_y);
  m_tree->Branch("pre_z",  &m_pre_z);
  m_tree->Branch("pre_r",  &m_pre_r);
  m_tree->Branch("post_x", &m_post_x);
  m_tree->Branch("post_y", &m_post_y);
  m_tree->Branch("post_z", &m_post_z);
  m_tree->Branch("post_r", &m_post_r);
  m_tree->Branch("mid_x",  &m_mid_x);
  m_tree->Branch("mid_y",  &m_mid_y);
  m_tree->Branch("mid_z",  &m_mid_z);
  m_tree->Branch("mid_r",  &m_mid_r);

  m_tree->Branch("pre_px",  &m_pre_px);
  m_tree->Branch("pre_py",  &m_pre_py);
  m_tree->Branch("pre_pz",  &m_pre_pz);
  m_tree->Branch("pre_p",   &m_pre_p);
  m_tree->Branch("pre_pT",  &m_pre_pT);
  m_tree->Branch("post_px", &m_post_px);
  m_tree->Branch("post_py", &m_post_py);
  m_tree->Branch("post_pz", &m_post_pz);
  m_tree->Branch("post_p",  &m_post_p);
  m_tree->Branch("post_pT", &m_post_pT);
  m_tree->Branch("pre_dir_x",  &m_pre_dir_x);
  m_tree->Branch("pre_dir_y",  &m_pre_dir_y);
  m_tree->Branch("pre_dir_z",  &m_pre_dir_z);
  m_tree->Branch("post_dir_x", &m_post_dir_x);
  m_tree->Branch("post_dir_y", &m_post_dir_y);
  m_tree->Branch("post_dir_z", &m_post_dir_z);
  m_tree->Branch("dp",      &m_dp);
  m_tree->Branch("loss",    &m_loss);
  m_tree->Branch("retained",&m_retained);

  m_tree->Branch("pre_ekin",  &m_pre_ekin);
  m_tree->Branch("post_ekin", &m_post_ekin);
  m_tree->Branch("dekin",     &m_dekin);
  m_tree->Branch("edep",      &m_edep);
  m_tree->Branch("nonion_edep", &m_nonion_edep);
  m_tree->Branch("step_length", &m_step_length);
  m_tree->Branch("material_radlen", &m_material_radlen);
  m_tree->Branch("step_tX0",   &m_step_tX0);
  m_tree->Branch("track_length_pre",  &m_track_length_pre);
  m_tree->Branch("track_length_post", &m_track_length_post);
  m_tree->Branch("global_time_pre",  &m_global_time_pre);
  m_tree->Branch("global_time_post", &m_global_time_post);

  m_tree->Branch("pre_volume",  &m_pre_volume);
  m_tree->Branch("post_volume", &m_post_volume);
  m_tree->Branch("pre_touchable_path",  &m_pre_touchable_path);
  m_tree->Branch("post_touchable_path", &m_post_touchable_path);
  m_tree->Branch("material",    &m_material);
  m_tree->Branch("process",     &m_process);

  if (m_recordDD4hepSurfaceIntervals.value()) {
    m_dd4hepTree = new TTree(
        "dd4hep_surface_tuple",
        "DD4hep midpoint-to-midpoint material intervals (one entry per event)");
    m_dd4hepTree->SetDirectory(m_file);
    m_dd4hepTree->Branch("run_id", &m_run_id);
    m_dd4hepTree->Branch("event_id", &m_event_id);
    m_dd4hepTree->Branch("interval_count", &m_dd4hep_interval_count);
    m_dd4hepTree->Branch("track_id", &m_dd4hep_track_id);
    m_dd4hepTree->Branch("parent_id", &m_dd4hep_parent_id);
    m_dd4hepTree->Branch("is_primary", &m_dd4hep_is_primary);
    m_dd4hepTree->Branch("pdg", &m_dd4hep_pdg);
    m_dd4hepTree->Branch("interval_index", &m_dd4hep_interval_index);
    m_dd4hepTree->Branch("from_track_step", &m_dd4hep_from_track_step);
    m_dd4hepTree->Branch("to_track_step", &m_dd4hep_to_track_step);
    m_dd4hepTree->Branch("segment_count", &m_dd4hep_segment_count);
    m_dd4hepTree->Branch("valid", &m_dd4hep_valid);
    m_dd4hepTree->Branch("g4_step_count", &m_dd4hep_g4_step_count);
    m_dd4hepTree->Branch("ebrem_step_count", &m_dd4hep_ebrem_step_count);
    m_dd4hepTree->Branch("from_x", &m_dd4hep_from_x);
    m_dd4hepTree->Branch("from_y", &m_dd4hep_from_y);
    m_dd4hepTree->Branch("from_z", &m_dd4hep_from_z);
    m_dd4hepTree->Branch("from_r", &m_dd4hep_from_r);
    m_dd4hepTree->Branch("to_x", &m_dd4hep_to_x);
    m_dd4hepTree->Branch("to_y", &m_dd4hep_to_y);
    m_dd4hepTree->Branch("to_z", &m_dd4hep_to_z);
    m_dd4hepTree->Branch("to_r", &m_dd4hep_to_r);
    m_dd4hepTree->Branch("from_track_length_mm",
                         &m_dd4hep_from_track_length_mm);
    m_dd4hepTree->Branch("to_track_length_mm",
                         &m_dd4hep_to_track_length_mm);
    m_dd4hepTree->Branch("path_length_mm", &m_dd4hep_path_length_mm);
    m_dd4hepTree->Branch("dd4hep_path_tX0", &m_dd4hep_path_tX0);
    m_dd4hepTree->Branch("g4_tX0", &m_dd4hep_g4_tX0);
    m_dd4hepTree->Branch("p_before_GeV", &m_dd4hep_p_before_GeV);
    m_dd4hepTree->Branch("p_after_GeV", &m_dd4hep_p_after_GeV);
    m_dd4hepTree->Branch("ebrem_loss_GeV", &m_dd4hep_ebrem_loss_GeV);
    m_dd4hepTree->Branch("surface_from", &m_dd4hep_surface_from);
    m_dd4hepTree->Branch("surface_to", &m_dd4hep_surface_to);
    m_dd4hepTree->Branch("dd4hep_materials", &m_dd4hep_materials);
  }
}

void GsfMaterialStepRecorderAnaElemTool::clearVectors() {
  m_track_id.clear();        m_parent_id.clear();
  m_is_primary.clear();      m_track_step_number.clear();
  m_recorded_step_index.clear();
  m_pdg.clear();             m_charge.clear();
  m_step_status_pre.clear();  m_step_status_post.clear();
  m_process_subtype.clear();
  m_pre_volume_copy_no.clear(); m_post_volume_copy_no.clear();
  m_pre_sensitive.clear(); m_post_sensitive.clear();

  m_pre_x.clear();  m_pre_y.clear();  m_pre_z.clear();  m_pre_r.clear();
  m_post_x.clear(); m_post_y.clear(); m_post_z.clear(); m_post_r.clear();
  m_mid_x.clear();  m_mid_y.clear();  m_mid_z.clear();  m_mid_r.clear();

  m_pre_px.clear();  m_pre_py.clear();  m_pre_pz.clear();  m_pre_p.clear();  m_pre_pT.clear();
  m_post_px.clear(); m_post_py.clear(); m_post_pz.clear(); m_post_p.clear(); m_post_pT.clear();
  m_pre_dir_x.clear(); m_pre_dir_y.clear(); m_pre_dir_z.clear();
  m_post_dir_x.clear(); m_post_dir_y.clear(); m_post_dir_z.clear();
  m_dp.clear(); m_loss.clear(); m_retained.clear();

  m_pre_ekin.clear(); m_post_ekin.clear(); m_dekin.clear();
  m_edep.clear(); m_nonion_edep.clear();
  m_step_length.clear(); m_material_radlen.clear(); m_step_tX0.clear();
  m_track_length_pre.clear(); m_track_length_post.clear();
  m_global_time_pre.clear(); m_global_time_post.clear();

  m_pre_volume.clear();  m_post_volume.clear();
  m_pre_touchable_path.clear(); m_post_touchable_path.clear();
  m_material.clear();    m_process.clear();

  m_dd4hep_interval_count = 0;
  m_dd4hep_track_id.clear(); m_dd4hep_parent_id.clear();
  m_dd4hep_is_primary.clear(); m_dd4hep_pdg.clear();
  m_dd4hep_interval_index.clear();
  m_dd4hep_from_track_step.clear(); m_dd4hep_to_track_step.clear();
  m_dd4hep_segment_count.clear(); m_dd4hep_valid.clear();
  m_dd4hep_g4_step_count.clear(); m_dd4hep_ebrem_step_count.clear();
  m_dd4hep_from_x.clear(); m_dd4hep_from_y.clear();
  m_dd4hep_from_z.clear(); m_dd4hep_from_r.clear();
  m_dd4hep_to_x.clear(); m_dd4hep_to_y.clear();
  m_dd4hep_to_z.clear(); m_dd4hep_to_r.clear();
  m_dd4hep_from_track_length_mm.clear();
  m_dd4hep_to_track_length_mm.clear();
  m_dd4hep_path_length_mm.clear(); m_dd4hep_path_tX0.clear();
  m_dd4hep_g4_tX0.clear();
  m_dd4hep_p_before_GeV.clear(); m_dd4hep_p_after_GeV.clear();
  m_dd4hep_ebrem_loss_GeV.clear();
  m_dd4hep_surface_from.clear(); m_dd4hep_surface_to.clear();
  m_dd4hep_materials.clear();
}
