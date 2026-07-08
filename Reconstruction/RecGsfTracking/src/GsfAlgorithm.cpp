#include "GsfAlgorithm.h"
#include "GsfComponent.h"
#include "GsfMixture.h"
#include "BetheHeitlerSplitter.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"

#include "kaltest/TKalDetCradle.h"
#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/THelicalTrack.h"
#include "kaltest/KalTrackDim.h"
#include "kaltest/TVSurface.h"

#include "DDKalTest/DDVMeasLayer.h"
#include "DDKalTest/DDVTrackHit.h"
#include "DDKalTest/DDCylinderMeasLayer.h"
#include "DDKalTest/DDCylinderHit.h"
#include "DDKalTest/DDPlanarHit.h"

#include "DDKalTest/DDKalDetector.h"

#include "edm4hep/TrackerHit.h"
#include "edm4hep/MCParticle.h"

#include <boost/format.hpp>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <exception>

DECLARE_COMPONENT(RecGsfTracking)

using DH = edm4hep::TrackState;

// ============================================================================
// Helpers
// ============================================================================

/// LCIO → KalTest parameter conversion
struct LcioSeed {
  double omega, d0, z0, phi, tanl;
  double refX, refY, refZ;
};

static LcioSeed extractSeed(const edm4hep::Track& trk) {
  for (auto loc : {DH::AtFirstHit, DH::AtIP}) {
    for (const auto& ts : trk.getTrackStates())
      if (ts.location == loc)
        return {ts.omega, ts.D0, ts.Z0, ts.phi, ts.tanLambda,
                ts.referencePoint.x, ts.referencePoint.y, ts.referencePoint.z};
  }
  return {};
}

/// Hit matched to a measurement layer
struct MatchedHit {
  edm4hep::TrackerHit lcioHit;
  const DDVMeasLayer* layer;
  DDVTrackHit*        kalHit;
  double              radius;
};

/// Find the measurement layer for a TrackerHit, first by cellID-indexed lookup
/// (O(1) via IsOnSurface), then fall back to the legacy radius-based scan.
static const DDVMeasLayer* findLayer(
    const edm4hep::TrackerHit& th,
    const std::multimap<int, const DDVMeasLayer*>& cellIDIndex,
    const TKalDetCradle* cradle) {

  TVector3 pos(th.getPosition().x, th.getPosition().y, th.getPosition().z);

  // ── primary: cellID-based lookup ──
  auto range = cellIDIndex.equal_range(th.getCellID());
  for (auto it = range.first; it != range.second; ++it) {
    auto* ml = it->second;
    auto* sf = dynamic_cast<const TVSurface*>(ml);
    if (sf && sf->IsOnSurface(pos)) return ml;
  }

  // ── fallback: radius-based scan over all layers ──
  double hr = pos.Perp();
  const DDVMeasLayer* best = nullptr;
  double bestDist = 1e18;

  for (int i = 0; i < cradle->GetEntriesFast(); i++) {
    auto* ml = dynamic_cast<DDVMeasLayer*>(cradle->At(i));
    if (!ml) continue;
    auto* sf = dynamic_cast<const TVSurface*>(ml);
    if (!sf) continue;

    if (sf->IsOnSurface(pos)) return ml;

    double s = sf->GetSortingPolicy();
    double d2 = (s - hr) * (s - hr);
    if (d2 < bestDist && d2 < 25.0) {
      bestDist = d2;
      best = ml;
    }
  }
  return best;
}

/// Build initial TKalTrack site, at the innermost hit.
static TKalTrackSite* makeInitialSite(
    const LcioSeed& seed, const MatchedHit& firstHit, double bz,
    double kappaSeed, double kappaCov) {

  TVector3 pivot = firstHit.layer->HitToXv(*firstHit.kalHit);
  THelicalTrack helix(-seed.d0, seed.phi - M_PI / 2., kappaSeed,
                       seed.z0, seed.tanl,
                       pivot.X(), pivot.Y(), pivot.Z(), bz);

  TKalMatrix sv(kSdim, 1);
  sv(0, 0) = helix.GetDrho();
  sv(1, 0) = helix.GetPhi0();
  sv(2, 0) = helix.GetKappa();
  sv(3, 0) = helix.GetDz();
  sv(4, 0) = helix.GetTanLambda();
  sv(5, 0) = 0.0;

  TKalMatrix cv(kSdim, kSdim);
  cv.Zero();
  cv(0, 0) = 100.0;
  cv(1, 1) = 0.01;
  cv(2, 2) = kappaCov;
  cv(3, 3) = 100.0;
  cv(4, 4) = 0.01;
  cv(5, 5) = 1e6;

  // Clone hit with huge errors → dummy initial site
  TVTrackHit* dummyHit = nullptr;
  if (auto* ch = dynamic_cast<DDCylinderHit*>(firstHit.kalHit))
    dummyHit = new DDCylinderHit(*ch);
  else if (auto* ph = dynamic_cast<DDPlanarHit*>(firstHit.kalHit))
    dummyHit = new DDPlanarHit(*ph);

  dummyHit->operator()(0, 1) = 1e16;
  if (dummyHit->GetDimension() > 1)
    dummyHit->operator()(1, 1) = 1e16;

  auto& site = *new TKalTrackSite(*dummyHit, kSdim);
  site.SetHitOwner();
  site.SetOwner();
  site.SetPivot(pivot);

  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kPredicted));
  site.Add(new TKalTrackState(sv, cv, site, TVKalSite::kFiltered));

  return &site;
}

/// Pure geometric extrapolation: innermost state → IP via helix move.
/// Covariance is propagated through the MoveTo Jacobian: C' = F·C·Fᵀ.
/// No material effects (MS / energy loss) applied.
static void extrapolateToIP_geometric(GsfComponent* comp,
                                      const DDCylinderMeasLayer* ipLayer,
                                      double /*bz*/,
                                      THelicalTrack& outHelix, TMatrixD& outCov) {
  auto& innerSite = *dynamic_cast<const TKalTrackSite*>(comp->kaltrack->At(1));
  auto& innerState = dynamic_cast<TKalTrackState&>(innerSite.GetCurState());

  outCov.ResizeTo(5, 5);
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      outCov(i, j) = innerState.GetCovMat()(i, j);

  outHelix = innerState.GetHelix();

  TVector3 ipPoint(0, 0, 0);
  if (ipLayer && ipLayer != &innerSite.GetHit().GetMeasLayer()) {
    double dphi;
    TMatrixD jac(5, 5);
    jac.UnitMatrix();
    outHelix.MoveTo(ipPoint, dphi, &jac, &outCov);  // jac=F, outCov=F·C·Fᵀ
  }
}

/// Full material-aware extrapolation: innermost state → IP using cradle Transport.
/// Includes multiple scattering and energy-loss correction via tQ and tSv.
/// MSOn / ElossOn must be set on the cradle for effects to be active.
static void extrapolateToIP_material(GsfComponent* comp,
                                     TKalDetCradle* cradle,
                                     const DDCylinderMeasLayer* ipLayer,
                                     double bz,
                                     THelicalTrack& outHelix, TMatrixD& outCov) {
  auto& innerSite = *dynamic_cast<const TKalTrackSite*>(comp->kaltrack->At(1));
  auto& innerState = dynamic_cast<TKalTrackState&>(innerSite.GetCurState());

  TKalMatrix tF(kSdim, kSdim);
  tF.UnitMatrix();
  TKalMatrix tQ(kSdim, kSdim);
  tQ.Zero();
  TKalMatrix tSv(kSdim, 1);
  TVector3 tX0;

  cradle->Transport(innerSite, *ipLayer, tX0, tSv, tF, tQ);

  // Propagate covariance: C' = F·C·Fᵀ + Q
  TKalMatrix kCov = innerState.GetCovMat();
  TKalMatrix tFt(TMatrixD::kTransposed, tF);
  kCov = tF * kCov * tFt + tQ;

  outCov.ResizeTo(5, 5);
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      outCov(i, j) = kCov(i, j);

  // tSv is the energy-loss-corrected state at IP (already a 5-d helix, +1)
  outHelix = THelicalTrack(tSv, TVector3(0, 0, 0), bz);
}

static void helixToMean(const THelicalTrack& h, TMatrixD& mean) {
  mean.ResizeTo(5, 1);
  mean(0, 0) = h.GetDrho();
  mean(1, 0) = h.GetPhi0();
  mean(2, 0) = h.GetKappa();
  mean(3, 0) = h.GetDz();
  mean(4, 0) = h.GetTanLambda();
}

static void wrapPhiNear(double& phi, double ref) {
  while (phi - ref > M_PI) phi -= 2.0 * M_PI;
  while (phi - ref < -M_PI) phi += 2.0 * M_PI;
}

static bool extrapolateToIP_component(GsfComponent* comp,
                                      bool materialIPExtrap,
                                      TKalDetCradle* cradle,
                                      const DDCylinderMeasLayer* ipLayer,
                                      double bz,
                                      THelicalTrack& outHelix,
                                      TMatrixD& outCov) {
  if (!comp || !comp->kaltrack || comp->kaltrack->GetEntriesFast() <= 1)
    return false;
  if (materialIPExtrap)
    extrapolateToIP_material(comp, cradle, ipLayer, bz, outHelix, outCov);
  else
    extrapolateToIP_geometric(comp, ipLayer, bz, outHelix, outCov);
  return true;
}

static bool weightedMixtureAtIP(const std::vector<GsfComponent*>& comps,
                                bool materialIPExtrap,
                                TKalDetCradle* cradle,
                                const DDCylinderMeasLayer* ipLayer,
                                double bz,
                                THelicalTrack& outHelix,
                                TMatrixD& outCov) {
  std::vector<double> weights;
  std::vector<TMatrixD> means;
  std::vector<TMatrixD> covs;
  weights.reserve(comps.size());
  means.reserve(comps.size());
  covs.reserve(comps.size());

  double sumW = 0.0;
  double phiRef = 0.0;
  bool havePhiRef = false;

  for (auto* comp : comps) {
    if (!comp || comp->weight <= 0.0) continue;
    THelicalTrack h(TMatrixD(5,1), TVector3(0, 0, 0), bz);
    TMatrixD cov(5, 5);
    if (!extrapolateToIP_component(comp, materialIPExtrap, cradle, ipLayer, bz, h, cov))
      continue;

    TMatrixD mean(5, 1);
    helixToMean(h, mean);
    if (!havePhiRef) {
      phiRef = mean(1, 0);
      havePhiRef = true;
    } else {
      wrapPhiNear(mean(1, 0), phiRef);
    }

    weights.push_back(comp->weight);
    means.push_back(mean);
    covs.push_back(cov);
    sumW += comp->weight;
  }

  if (weights.empty() || sumW <= 0.0) return false;

  TMatrixD mergedMean(5, 1);
  mergedMean.Zero();
  for (size_t i = 0; i < weights.size(); i++) {
    TMatrixD term = means[i];
    term *= weights[i] / sumW;
    mergedMean += term;
  }
  wrapPhiNear(mergedMean(1, 0), 0.0);

  TMatrixD mergedCov(5, 5);
  mergedCov.Zero();
  for (size_t i = 0; i < weights.size(); i++) {
    const double w = weights[i] / sumW;
    TMatrixD d = means[i] - mergedMean;
    wrapPhiNear(d(1, 0), 0.0);
    TMatrixD dT(TMatrixD::kTransposed, d);
    TMatrixD term = covs[i] + d * dT;
    term *= w;
    mergedCov += term;
  }

  outCov.ResizeTo(5, 5);
  outCov = mergedCov;
  outHelix = THelicalTrack(mergedMean, TVector3(0, 0, 0), bz);
  return true;
}

/// Fill an edm4hep TrackState from a THelicalTrack + 5x5 cov
static void fillTrackState(edm4hep::TrackState& ts,
                            const THelicalTrack& h,
                            const TMatrixD& cov, double bz) {

  double d0 = -h.GetDrho();
  double phi = h.GetPhi0() + M_PI / 2.;
  double omega = (bz != 0) ? (h.GetKappa() * bz * 2.99792458e-4) : 0.0;
  double z0 = h.GetDz();
  double tanl = h.GetTanLambda();

  while (phi > M_PI) phi -= 2 * M_PI;
  while (phi < -M_PI) phi += 2 * M_PI;

  double al = omega / h.GetKappa();

  ts.D0 = (float)d0;
  ts.phi = (float)phi;
  ts.omega = (float)omega;
  ts.Z0 = (float)z0;
  ts.tanLambda = (float)tanl;

  decltype(ts.covMatrix) cl{};
  cl[ 0] = cov(0, 0);   cl[ 1] = -cov(1, 0);  cl[ 2] = cov(1, 1);
  cl[ 3] = -cov(2, 0) * al;  cl[ 4] = cov(2, 1) * al;
  cl[ 5] = cov(2, 2) * al * al;
  cl[ 6] = -cov(3, 0);       cl[ 7] = cov(3, 1);
  cl[ 8] = cov(3, 2) * al;   cl[ 9] = cov(3, 3);
  cl[10] = -cov(4, 0);       cl[11] = cov(4, 1);
  cl[12] = cov(4, 2) * al;   cl[13] = cov(4, 3);
  cl[14] = cov(4, 4);
  ts.covMatrix = cl;

  float pv[3] = {(float)h.GetPivot().X(),
                 (float)h.GetPivot().Y(),
                 (float)h.GetPivot().Z()};
  ts.referencePoint = edm4hep::Vector3f(pv);
}

/// Compute t/X0 for this layer (inner + outer material)
static double thicknessInX0(const DDVMeasLayer* layer) {
  double tX0 = 0;
  if (!layer->surface()) return 0;

  auto& matIn = layer->GetMaterial(false);
  auto& matOut = layer->GetMaterial(true);

  if (matIn.GetRadLength() > 0)
    tX0 += layer->surface()->innerThickness() / matIn.GetRadLength();
  if (matOut.GetRadLength() > 0)
    tX0 += layer->surface()->outerThickness() / matOut.GetRadLength();

  return tX0;
}

// ============================================================================
// Algorithm
// ============================================================================

RecGsfTracking::RecGsfTracking(const std::string& name,
                                         ISvcLocator* svc)
  : Algorithm(name, svc) {}

StatusCode RecGsfTracking::initialize() {
  m_nEvt = 0;

  m_geosvc = service<IGeomSvc>("GeomSvc");
  m_field = m_geosvc->lcdd()
                ->field()
                .magneticField(dd4hep::Position(0, 0, 0))
                .z() / dd4hep::tesla;
  info() << "B=" << m_field << " T" << endmsg;

  // Build DDKalTest geometry cradle (standalone, independent of tracking pipeline)
  m_cradle = new TKalDetCradle();
  m_cradle->SetOwner(true);

  auto det = m_geosvc->lcdd();
  std::vector<dd4hep::DetElement> detectors = det->detectors("tracker");
  for (auto& pd : det->detectors("passive")) detectors.push_back(pd);
  for (auto& c : det->detectors("calorimeter")) {
    std::string n = c.name();
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    if (n.find("ecal") != std::string::npos) detectors.push_back(c);
  }

  double minS = 1e99;
  for (auto& de : detectors) {
    auto* kd = new DDKalDetector(de);
    m_detectors.push_back(kd);
    m_cradle->Install(*kd);
    for (int i = 0; i < kd->GetEntriesFast(); i++) {
      auto* s = dynamic_cast<const TVSurface*>(kd->At(i));
      if (!s) continue;
      if (s->GetSortingPolicy() < minS &&
          dynamic_cast<DDCylinderMeasLayer*>(kd->At(i))) {
        minS = s->GetSortingPolicy();
        m_ipLayer = dynamic_cast<DDCylinderMeasLayer*>(kd->At(i));
      }
    }
  }
  m_cradle->Close();
  if (m_doMS) m_cradle->SwitchOnMS(); else m_cradle->SwitchOffMS();
  if (m_doDEDX) m_cradle->SwitchOnDEDX(); else m_cradle->SwitchOffDEDX();

  // ── build cellID → layer index (same approach as MarlinDDKalTest) ──
  m_cellIDToLayer.clear();
  for (auto* kd : m_detectors) {
    int nLayers = kd->GetEntriesFast();
    for (int i = 0; i < nLayers; i++) {
      auto* ml = dynamic_cast<const DDVMeasLayer*>(kd->At(i));
      if (!ml) continue;
      if (!ml->IsActive()) continue;
      for (auto cid : ml->getCellIDs())
        m_cellIDToLayer.insert({cid, ml});
    }
  }

  info() << "Cradle: " << m_cradle->GetEntriesFast() << " layers"
         << "  cellID-index: " << m_cellIDToLayer.size() << " entries"
         << " IP r=" << (m_ipLayer ? m_ipLayer->GetR() : -1) << endmsg;

  try {
    const auto bhModel = BetheHeitlerSplitter::modelFromName(m_bhModel.value());
    info() << "Bethe-Heitler model: "
           << BetheHeitlerSplitter::modelName(bhModel) << endmsg;
  } catch (const std::exception& e) {
    error() << e.what() << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

// ---------------------------------------------------------------------------
StatusCode RecGsfTracking::execute() {
  m_nEvt++;
  const int eventIndex = m_nEvt - 1;
  const bool selectedOnly = !m_selectedEventIndices.value().empty();
  if (selectedOnly && std::find(m_selectedEventIndices.value().begin(),
                                m_selectedEventIndices.value().end(),
                                eventIndex) == m_selectedEventIndices.value().end()) {
    m_outputTracks.createAndPut();
    return StatusCode::SUCCESS;
  }

  info() << "GSF event index " << eventIndex
         << " (event count " << m_nEvt << ")" << endmsg;
  const auto* in = m_inputTracks.get();
  if (!in) return StatusCode::SUCCESS;

  auto* out = m_outputTracks.createAndPut();
  int nFit = 0;
  double bz = m_field;

  for (const auto& trk : *in) {
    auto assocHits = trk.getTrackerHits();
    if (assocHits.size() < 5) continue;

    // ---- Step 1: seed from LCIO ----
    LcioSeed seed = extractSeed(trk);
    if (seed.omega == 0) continue;
    int charge = (seed.omega > 0) ? 1 : -1;

    // ---- Step 2: match hits to measurement layers ----
    std::vector<MatchedHit> hits;
    for (const auto& th : assocHits) {
      if (!th.isAvailable()) continue;

      TVector3 pos(th.getPosition().x, th.getPosition().y,
                    th.getPosition().z);

      auto* layer = findLayer(th, m_cellIDToLayer, m_cradle);
      if (!layer) continue;

      DDVTrackHit* khit = layer->ConvertLCIOTrkHit(
          const_cast<edm4hep::TrackerHit&>(th));
      if (!khit) continue;

      hits.push_back({th, layer, khit,
                      std::hypot(pos.X(), pos.Y())});
    }

    std::sort(hits.begin(), hits.end(),
              [](auto& a, auto& b) { return a.radius < b.radius; });
    if (hits.empty()) continue;

    // ---- Step 3: compute kappa seed ----
    double alpha = bz * 2.99792458e-4;
    double kappaSeed = (bz != 0) ? (seed.omega / alpha) : 1e-5;

    // ---- Step 4: forward GSF filter ----
    auto* site = makeInitialSite(seed, hits[0], bz, kappaSeed, m_kappaSeedCov);
    auto* initComp = new GsfComponent();
    initComp->weight = 1.0;
    initComp->charge = charge;
    initComp->kaltrack = new TKalTrack();
    initComp->kaltrack->SetOwner();
    initComp->kaltrack->Add(site);

    std::vector<GsfComponent*> comps = {initComp};
    int nProc = 0, nSplits = 0, nReductions = 0, maxCompsEver = 1;
    double totalTX0 = 0.0, maxTX0Layer = 0.0;
    bool justSplit = false;

    for (size_t ih = 1; ih < hits.size(); ih++) {
      auto& hi = hits[ih];
      // measurement position (same for all components)
      TVector3 measPos = hi.layer->HitToXv(*hi.kalHit);

      const double stepTX0 = thicknessInX0(hi.layer);
      totalTX0 += stepTX0;
      if (stepTX0 > maxTX0Layer) maxTX0Layer = stepTX0;

      // Apply the material/BH process before the measurement update at this hit.
      // The current coarse material estimate is attached to the target layer, so
      // the child hypotheses are what the hit likelihood sees.
      justSplit = false;
      if (stepTX0 > m_bhSplitThresh && m_isElectron &&
          (int)comps.size() < m_maxComponents) {
        if (m_verboseDump && m_verboseSplitDump) {
          info() << boost::format("  ── BH Split before hit %d (r=%.1f mm, step tX0=%.2e) — %d comps before split")
                    % ih % hi.radius % stepTX0 % (int)comps.size() << endmsg;
        }
        BetheHeitlerSplitter bhs(m_bhModel.value());
        std::vector<GsfComponent*> newCps;
        for (auto* comp : comps) {
          double parentKappa = comp->helixAtLastSite(bz).GetKappa();
          auto children = bhs.split(comp, stepTX0, bz);
          if (m_verboseDump && m_verboseSplitDump) {
            double parentPT = (bz != 0 && parentKappa != 0) ? 1.0/std::abs(parentKappa) : 0;
            info() << boost::format("    parent κ=%.4e (pT≈%.3f)  weight=%.4f  → %d children")
                      % parentKappa % parentPT % comp->weight % (int)children.size() << endmsg;
            for (size_t ci = 0; ci < children.size(); ci++) {
              double childKappa = children[ci]->helixAtLastSite(bz).GetKappa();
              double childPT = (bz != 0 && childKappa != 0) ? 1.0/std::abs(childKappa) : 0;
              info() << boost::format("      child[%d] κ=%.4e (pT≈%.3f)  weight=%.4f")
                        % ci % childKappa % childPT % children[ci]->weight << endmsg;
            }
          }
          for (auto* c : children) newCps.push_back(c);
        }
        comps = std::move(newCps);
        nSplits++;
        justSplit = true;
      }
      GsfMixture::normalizeWeights(comps);
      if ((int)comps.size() > maxCompsEver) maxCompsEver = (int)comps.size();
      if ((int)comps.size() > m_maxComponents) {
        GsfMixture::reduce(comps, m_maxComponents, bz);
        nReductions++;
      }
      GsfMixture::normalizeWeights(comps);

      std::vector<GsfComponent*> accepted;
      std::vector<double> dchi2s;
      if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
        dchi2s.reserve(comps.size());
      }
      for (auto* comp : comps) {
        DDVTrackHit* khClone = nullptr;
        if (auto* ch = dynamic_cast<DDCylinderHit*>(hi.kalHit))
          khClone = new DDCylinderHit(*ch);
        else if (auto* ph = dynamic_cast<DDPlanarHit*>(hi.kalHit))
          khClone = new DDPlanarHit(*ph);
        else continue;

        auto* st = new TKalTrackSite(*khClone, kSdim);
        st->SetHitOwner();

        if (comp->kaltrack->AddAndFilter(*st)) {
          double dchi = st->GetDeltaChi2();
          comp->weight *= std::exp(-0.5 * std::min(dchi, 100.0));
          accepted.push_back(comp);
          if (m_verboseDump && m_verboseSplitDump && (justSplit || (int)comps.size() > 1)) {
            dchi2s.push_back(dchi);
          }
        } else {
          bool recovered = false;
          // KalTest can fail re-crossing after Transport has already pivoted exactly to the hit.
          // In that narrow case keep the predicted state instead of dropping the component.
          if (st->GetEntriesFast() > TVKalSite::kPredicted) {
            auto& preState = dynamic_cast<const TKalTrackState&>(st->GetState(TVKalSite::kPredicted));
            auto preHel = preState.GetHelix();
            const auto& pv = preHel.GetPivot();
            const double pivotResidual = std::sqrt((pv.X() - measPos.X()) * (pv.X() - measPos.X()) +
                                                   (pv.Y() - measPos.Y()) * (pv.Y() - measPos.Y()) +
                                                   (pv.Z() - measPos.Z()) * (pv.Z() - measPos.Z()));
            if (pivotResidual < 1e-3) {
              st->Add(new TKalTrackState(preState, preState.GetCovMat(), *st, TVKalSite::kFiltered));
              st->SetOwner();
              comp->kaltrack->Add(st);
              accepted.push_back(comp);
              recovered = true;
            }
          }
          if (!recovered) {
            delete st;
            delete comp;
          }
        }
      }

      if (accepted.empty()) {
        warning() << boost::format("GSF event index %d track %d: all components rejected at hit %d (r=%.1f mm); no GSF output track")
                     % (m_nEvt - 1) % (nFit + 1) % ih % hi.radius << endmsg;
        comps.clear();
        break;
      }

      // ── verbose: prediction vs measurement after pre-hit split ──
      if (m_verboseDump && m_verboseSplitDump && justSplit && !accepted.empty()) {
        info() << boost::format("  >>> Post-split measurement @ hit %d (r=%.1f mm) — %d components survive")
                  % ih % hi.radius % (int)accepted.size() << endmsg;
        info() << boost::format("      measurement: (%.3f, %.3f, %.3f)")
                  % measPos.X() % measPos.Y() % measPos.Z() << endmsg;
        for (size_t ci = 0; ci < accepted.size(); ci++) {
          auto* c = accepted[ci];
          double k = c->helixAtLastSite(bz).GetKappa();
          double pt = (bz != 0 && k != 0) ? std::abs(alpha / k) : 0;
          double dchi = (ci < dchi2s.size()) ? dchi2s[ci] : -1;
          info() << boost::format("      comp[%d] κ=%.4e pT≈%.3f  weight=%.4f  Δχ²=%.1f")
                    % ci % k % (1.0/std::abs(k)) % c->weight % dchi << endmsg;
        }
      }

      comps = std::move(accepted);
      GsfMixture::normalizeWeights(comps);
      nProc++;
    }

    // ---- Step 5: smooth, extrapolate to IP, write output ----
    if (!comps.empty() && nProc > 0) {
      for (auto* c : comps)
        if (c->kaltrack->GetEntriesFast() > 1)
          c->kaltrack->SmoothAll();

      GsfMixture::normalizeWeights(comps);

      // Pick best component among components with at least one real filtered hit.
      int bestIdx = -1;
      for (size_t i = 0; i < comps.size(); i++) {
        if (!comps[i]->kaltrack || comps[i]->kaltrack->GetEntriesFast() <= 1) continue;
        if (bestIdx < 0 || comps[i]->weight > comps[bestIdx]->weight)
          bestIdx = (int)i;
      }
      if (bestIdx < 0) {
        warning() << "GSF event index " << (m_nEvt - 1)
                  << ": no component has a filtered hit; no GSF output track" << endmsg;
        for (auto* c : comps) delete c;
        for (auto& h : hits) delete h.kalHit;
        continue;
      }

      auto* best = comps[bestIdx];

      if (m_verboseDump && m_verboseSplitDump) {
        auto dumpSite = [&](const char* label, int idx) {
          if (!best->kaltrack || idx < 0 || idx >= best->kaltrack->GetEntriesFast()) return;
          auto* site = dynamic_cast<const TKalTrackSite*>(best->kaltrack->At(idx));
          if (!site) return;
          auto& state = dynamic_cast<TKalTrackState&>(site->GetCurState());
          auto h = state.GetHelix();
          auto pv = h.GetPivot();
          info() << boost::format("  DIAG %s site=%d drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                    % label % idx % h.GetDrho() % h.GetPhi0() % h.GetKappa() % h.GetDz() % h.GetTanLambda()
                    % pv.X() % pv.Y() % pv.Z() << endmsg;
        };
        dumpSite("initial", 0);
        dumpSite("inner", 1);
        dumpSite("last", best->kaltrack->GetEntriesFast() - 1);
      }

      // Extrapolate to IP (method selected by MaterialIPExtrapolation).
      // By default publish the highest-weight branch. Optionally publish the
      // moment-matched weighted mixture at IP.
      THelicalTrack bestIpHelix(TMatrixD(5,1), TVector3(0, 0, 0), bz);
      TMatrixD bestIpCov(5, 5);
      extrapolateToIP_component(best, m_materialIPExtrap, m_cradle, m_ipLayer,
                                bz, bestIpHelix, bestIpCov);

      THelicalTrack ipHelix = bestIpHelix;
      TMatrixD ipCov = bestIpCov;
      const std::string outputMode = m_outputMode.value();
      bool usedWeightedOutput = false;
      if (outputMode == "WeightedMean") {
        THelicalTrack mixIpHelix(TMatrixD(5,1), TVector3(0, 0, 0), bz);
        TMatrixD mixIpCov(5, 5);
        if (weightedMixtureAtIP(comps, m_materialIPExtrap, m_cradle, m_ipLayer,
                                bz, mixIpHelix, mixIpCov)) {
          ipHelix = mixIpHelix;
          ipCov = mixIpCov;
          usedWeightedOutput = true;
        } else {
          warning() << "GSFOutputMode=WeightedMean failed; falling back to BestBranch"
                    << endmsg;
        }
      } else if (outputMode != "BestBranch") {
        warning() << "Unknown GSFOutputMode '" << outputMode
                  << "'; falling back to BestBranch" << endmsg;
      }
      if (m_verboseDump && m_verboseSplitDump) {
        auto pv = ipHelix.GetPivot();
        info() << boost::format("  DIAG ip    drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                  % ipHelix.GetDrho() % ipHelix.GetPhi0() % ipHelix.GetKappa() % ipHelix.GetDz() % ipHelix.GetTanLambda()
                  % pv.X() % pv.Y() % pv.Z() << endmsg;
        if (usedWeightedOutput) {
          auto bestPv = bestIpHelix.GetPivot();
          info() << boost::format("  DIAG best-ip drho=%.6g phi0=%.6g kappa=%.6g dz=%.6g tanl=%.6g pivot=(%.3f, %.3f, %.3f)")
                    % bestIpHelix.GetDrho() % bestIpHelix.GetPhi0() % bestIpHelix.GetKappa()
                    % bestIpHelix.GetDz() % bestIpHelix.GetTanLambda()
                    % bestPv.X() % bestPv.Y() % bestPv.Z() << endmsg;
        }
      }

      // Write output track
      auto ot = out->create();
      ot.setType(2);
      ot.setChi2(best->kaltrack->GetChi2());
      ot.setNdf(best->kaltrack->GetNDF());

      edm4hep::TrackState ts;
      ts.location = DH::AtIP;
      fillTrackState(ts, ipHelix, ipCov, bz);
      ot.addToTrackStates(ts);

      for (const auto& h : assocHits) ot.addToTrackerHits(h);

      // ── MC truth ──
      double t_pT = 0, t_eta = 0, t_phi = 0, t_p = 0;
      if (m_mcParticles.isValid()) {
        auto* mcCol = m_mcParticles.get();
        if (mcCol && mcCol->size() > 0) {
          auto mcp = (*mcCol)[0];  // first particle = primary electron
          auto& mom = mcp.getMomentum();
          t_pT  = std::hypot(mom.x, mom.y);
          t_p   = std::hypot(t_pT, mom.z);
          t_eta = (t_p > 0 && std::abs(t_pT / t_p) < 1.0)
                  ? std::atanh(mom.z / t_p) : 0.0;
          t_phi = std::atan2(mom.y, mom.x);
        }
      }

      // ── fill TrackSummary ──
      auto toPtEta = [bz](double omega, double tanl) {
        double a = bz * 2.99792458e-4;
        double pT = (omega != 0) ? std::abs(a / omega) : 0.0;
        double eta = std::asinh(tanl);
        return std::make_pair(pT, eta);
      };
      double lcio_pT, lcio_eta, gsf_pT, gsf_eta;
      std::tie(lcio_pT, lcio_eta) = toPtEta(seed.omega, seed.tanl);
      std::tie( gsf_pT,  gsf_eta) = toPtEta(ts.omega, ts.tanLambda);
      double lcio_p = lcio_pT * std::cosh(lcio_eta);
      double  gsf_p =  gsf_pT * std::cosh( gsf_eta);

      TrackSummary sum;
      sum.iev      = m_nEvt;
      sum.charge   = charge;
      sum.nHits    = nProc + 1;
      sum.nComps   = (int)comps.size();
      sum.truth_pT = t_pT; sum.truth_eta = t_eta; sum.truth_phi = t_phi; sum.truth_p = t_p;
      sum.lcio_pT  = lcio_pT; sum.lcio_eta  = lcio_eta;  sum.lcio_phi  = seed.phi;
      sum.lcio_d0  = seed.d0; sum.lcio_z0   = seed.z0;   sum.lcio_p    = lcio_p;
      sum.lcio_chi2 = trk.getChi2(); sum.lcio_ndf = trk.getNdf();
      sum.gsf_pT   = gsf_pT;   sum.gsf_eta   = gsf_eta;   sum.gsf_phi   = ts.phi;
      sum.gsf_d0   = ts.D0;    sum.gsf_z0    = ts.Z0;     sum.gsf_p     = gsf_p;
      sum.gsf_chi2 = best->kaltrack->GetChi2(); sum.gsf_ndf = best->kaltrack->GetNDF();
      // GSF diagnostics
      sum.nSplits     = nSplits;
      sum.nReductions = nReductions;
      sum.maxCompsEver= maxCompsEver;
      sum.finalComps  = (int)comps.size();
      sum.bestWeight  = best->weight;
      sum.meanWeight  = 1.0 / (int)comps.size();
      sum.maxTX0Layer = maxTX0Layer;
      sum.totalTX0    = totalTX0;
      m_summaries.push_back(sum);

      // ── pretty-print (guarded by VerboseDump) ──
      if (m_verboseDump) {
        double dphi = ts.phi - seed.phi;
        while (dphi >  M_PI) dphi -= 2 * M_PI;
        while (dphi < -M_PI) dphi += 2 * M_PI;
        std::string const sep(60, '-');

        // ── parameter comparison table ──
        info() << sep << endmsg;
        info() << boost::format("%s %02d  |  comps %2d  hits %d/%d  q=%+d  p %.2f GeV  χ²/ndf %.1f/%d")
                  % "GSF Track" % (nFit + 1) % sum.nComps % sum.nHits % (int)hits.size()
                  % charge % gsf_p % best->kaltrack->GetChi2() % best->kaltrack->GetNDF() << endmsg;
        info() << sep << endmsg;
        info() << boost::format("  %-6s  %10s  %10s  %10s  %s") % ""    % "Truth"  % "LCIO"   % "GSF"    % "" << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f  %s")   % "pT"  % t_pT      % lcio_pT   % gsf_pT   % "GeV" << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f")       % "η"   % t_eta     % lcio_eta  % gsf_eta          << endmsg;
        info() << boost::format("  %-6s  %10.4f  %10.4f  %10.4f")       % "φ"   % t_phi     % seed.phi   % ts.phi           << endmsg;
        info() << boost::format("  %-6s  %10s  %10.4f  %10.4f  %s")     % "d0"  % "—"       % seed.d0    % ts.D0    % "mm"  << endmsg;
        info() << boost::format("  %-6s  %10s  %10.4f  %10.4f  %s")     % "z0"  % "—"       % seed.z0    % ts.Z0    % "mm"  << endmsg;
        info() << boost::format("  %-6s  %10.3f  %10.3f  %10.3f  %s")   % "p"   % t_p       % lcio_p     % gsf_p    % "GeV" << endmsg;
        info() << boost::format("  %-6s  %10s  %7.1f/%-2d  %7.1f/%-2d")
                  % "χ²/ndf" % "—" % trk.getChi2() % trk.getNdf()
                  % best->kaltrack->GetChi2() % best->kaltrack->GetNDF() << endmsg;

        // ── GSF diagnostics table ──
        info() << sep << endmsg;
        info() << boost::format("  GSF diagnostics | splits %d  reductions %d  peak-comps %d  final-comps %d")
                  % nSplits % nReductions % maxCompsEver % sum.finalComps << endmsg;
        info() << boost::format("  weights        | best %.4f  mean %.4f  ratio %.2f")
                  % best->weight % (1.0 / sum.finalComps) % (best->weight * sum.finalComps) << endmsg;
        info() << boost::format("  output         | mode %s")
                  % (usedWeightedOutput ? "WeightedMean" : "BestBranch") << endmsg;
        info() << boost::format("  material       | max-tX0 %.2e  total-tX0 %.2e")
                  % maxTX0Layer % totalTX0 << endmsg;
        info() << sep << endmsg;
      }

      nFit++;
    }

    for (auto* c : comps) delete c;
    for (auto& h : hits) delete h.kalHit;
  }

  info() << "Fitted: " << nFit << " / " << in->size() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode RecGsfTracking::finalize() {
  info() << "Processed " << m_nEvt << " events" << endmsg;
  m_detectors.clear();
  if (m_cradle) { m_cradle->SetOwner(true); delete m_cradle; m_cradle = nullptr; }
  return Algorithm::finalize();
}
