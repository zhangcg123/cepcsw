#include "GsfMixture.h"
#include "GsfComponent.h"

#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"

#include <cmath>
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <utility>

namespace GsfMixture {

// ============================================================================
void normalizeWeights(std::vector<GsfComponent*>& comps) {
  if (comps.empty()) return;
  double sum = 0.0;
  for (auto* c : comps) sum += c->weight;
  if (sum < 1e-30) {
    for (auto* c : comps) c->weight = 1.0 / comps.size();
    return;
  }
  for (auto* c : comps) c->weight /= sum;
}

void removeLowWeight(std::vector<GsfComponent*>& comps, double cutoff,
                     bool protectIdentity) {
  if (comps.empty() || cutoff <= 0.0) return;
  normalizeWeights(comps);
  auto* largest = *std::max_element(
      comps.begin(), comps.end(),
      [](const GsfComponent* a, const GsfComponent* b) {
        return a->weight < b->weight;
      });
  auto out = comps.begin();
  for (auto* component : comps) {
    if (component->weight >= cutoff || component == largest ||
        (protectIdentity && component->noRadiationLineage)) {
      *out++ = component;
    } else {
      delete component;
    }
  }
  comps.erase(out, comps.end());
  normalizeWeights(comps);
}

// ============================================================================
/// Full-covariance KL divergence between two multivariate Gaussians.
///   KL(P‖Q) = ½ [ ln(|ΣQ|/|ΣP|) - d + tr(ΣQ⁻¹ ΣP) + (μQ-μP)ᵀ ΣQ⁻¹ (μQ-μP) ]
static double klDirectional(GsfComponent* p, GsfComponent* q,
                             const TMatrixD& muP, const TMatrixD& muQ,
                             double bz) {
  TMatrixD covP = p->covAtLastSite(bz);
  TMatrixD covQ = q->covAtLastSite(bz);

  static constexpr int d = 5;
  // Determinants
  double detP = covP.Determinant();
  double detQ = covQ.Determinant();
  // Track covariances have mixed physical units, so their 5-D determinants
  // can legitimately be far below an absolute epsilon (typically 1e-22 here).
  // Reject only non-positive or non-finite matrices.
  if (!(detP > 0.0) || !(detQ > 0.0) ||
      !std::isfinite(detP) || !std::isfinite(detQ))
    return 1e30;

  double logDetP = std::log(detP);
  double logDetQ = std::log(detQ);

  // ΣQ⁻¹ ΣP  →  solve ΣQ * X = ΣP for X, then trace(X)
  TMatrixD invQ = covQ;
  invQ.Invert();  // in-place inversion
  if (invQ(0, 0) != invQ(0, 0))  // NaN check
    return 1e30;

  TMatrixD invQ_covP = invQ * covP;
  double traceInvQCovP = 0;
  for (int i = 0; i < d; i++)
    traceInvQCovP += invQ_covP(i, i);

  // Mahalanobis: (μQ - μP)ᵀ ΣQ⁻¹ (μQ - μP)
  TMatrixD dmu = muQ - muP;
  TMatrixD dmuT(TMatrixD::kTransposed, dmu);
  TMatrixD mahalMat = dmuT * invQ * dmu;
  double mahal = mahalMat(0, 0);

  double result = 0.5 * (logDetQ - logDetP - d + traceInvQCovP + mahal);
  if (result < 0) result = 0;  // numerical guard
  return result;
}

static double wrapNear(double value, double reference) {
  while (value - reference >= M_PI) value -= 2.0 * M_PI;
  while (value - reference < -M_PI) value += 2.0 * M_PI;
  return value;
}

static std::string boundedHistory(const std::string& history) {
  constexpr std::size_t maxLength = 4096;
  constexpr std::size_t edgeLength = 2000;
  if (history.size() <= maxLength) return history;
  return history.substr(0, edgeLength) + "...<history-truncated>..." +
      history.substr(history.size() - edgeLength);
}

static void mergeProcessModeFractions(
    std::map<std::pair<int, int>, double>& keep,
    const std::map<std::pair<int, int>, double>& drop,
    double keepFraction, double dropFraction) {
  if (keep.empty() && drop.empty()) return;
  for (auto& item : keep) item.second *= keepFraction;
  for (const auto& item : drop)
    keep[item.first] += dropFraction * item.second;
}

static void momentMerge(GsfComponent* keep, GsfComponent* drop, double bz) {
  const double totalWeight = keep->weight + drop->weight;
  if (totalWeight <= 0.0) return;

  const double dominantLineageWeight = std::max(
      keep->weight * keep->dominantLineageFraction,
      drop->weight * drop->dominantLineageFraction);

  const double wk = keep->weight / totalWeight;
  const double wd = drop->weight / totalWeight;
  const std::string keepHistory = boundedHistory(keep->debugHistory);
  const std::string dropHistory = boundedHistory(drop->debugHistory);
  const double mergedChi2 = wk * keep->fitChi2 + wd * drop->fitChi2;

  TMatrixD muK(5, 1), muD(5, 1);
  keep->helixAtLastSite(bz).PutInto(muK);
  drop->helixAtLastSite(bz).PutInto(muD);
  muD(1, 0) = wrapNear(muD(1, 0), muK(1, 0));

  TMatrixD mergedMu = muK;
  mergedMu *= wk;
  TMatrixD weightedDropMu = muD;
  weightedDropMu *= wd;
  mergedMu += weightedDropMu;

  const TMatrixD covK = keep->covAtLastSite(bz);
  const TMatrixD covD = drop->covAtLastSite(bz);
  TMatrixD dK = muK - mergedMu;
  TMatrixD dD = muD - mergedMu;
  TMatrixD dKT(TMatrixD::kTransposed, dK);
  TMatrixD dDT(TMatrixD::kTransposed, dD);
  TMatrixD mergedCov = covK + dK * dKT;
  mergedCov *= wk;
  TMatrixD dropTerm = covD + dD * dDT;
  dropTerm *= wd;
  mergedCov += dropTerm;

  // Only the common-surface continuation state is merged.  The retained
  // component's measurement history remains a real representative history.
  keep->setContinuationSurfaceState(mergedMu, mergedCov, bz);

  keep->weight = totalWeight;
  keep->dominantLineageFraction = dominantLineageWeight / totalWeight;
  mergeProcessModeFractions(keep->forwardProcessModeFractions,
                            drop->forwardProcessModeFractions, wk, wd);
  mergeProcessModeFractions(keep->reverseProcessModeFractions,
                            drop->reverseProcessModeFractions, wk, wd);
  for (auto& source : keep->smoothingSourceFractions)
    source.second *= wk;
  for (const auto& source : drop->smoothingSourceFractions)
    keep->smoothingSourceFractions[source.first] += wd * source.second;
  keep->fitChi2 = mergedChi2;
  if (!dropHistory.empty()) {
    keep->debugHistory = boundedHistory(
        "merge(" + keepHistory + " | " + dropHistory + ")");
  }
}

/// Symmetric KL distance: ½[KL(P‖Q) + KL(Q‖P)]
static double klDistance(GsfComponent* a, GsfComponent* b, double bz) {
  TMatrixD muA(5, 1), muB(5, 1);
  a->helixAtLastSite(bz).PutInto(muA);
  b->helixAtLastSite(bz).PutInto(muB);

  double klAB = klDirectional(a, b, muA, muB, bz);
  double klBA = klDirectional(b, a, muB, muA, bz);
  return 0.5 * (klAB + klBA);
}

// ============================================================================
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            bool protectIdentity) {
  reduce(comps, maxN, bz, protectIdentity, {});
}

void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            bool protectIdentity,
            const std::function<void(const std::string&)>& logger) {
  if (maxN < 1) maxN = 1;

  int mergeStep = 0;
  while ((int)comps.size() > maxN) {
    int bi = -1, bj = -1;
    double bestDist = 1e30;
    for (size_t i = 0; i < comps.size(); i++) {
      for (size_t j = i + 1; j < comps.size(); j++) {
        if (protectIdentity && maxN > 1 &&
            comps[i]->noRadiationLineage != comps[j]->noRadiationLineage)
          continue;
        double d = klDistance(comps[i], comps[j], bz);
        if (bi < 0 || d < bestDist) {
          bestDist = d;
          bi = (int)i;
          bj = (int)j;
        }
      }
    }
    if (bi < 0) break;

    const int origI = bi;
    const int origJ = bj;
    const double wi = comps[bi]->weight;
    const double wj = comps[bj]->weight;
    const int idI = comps[bi]->debugId;
    const int idJ = comps[bj]->debugId;
    const double ki = comps[bi]->helixAtLastSite(bz).GetKappa();
    const double kj = comps[bj]->helixAtLastSite(bz).GetKappa();
    const double detI = comps[bi]->covAtLastSite(bz).Determinant();
    const double detJ = comps[bj]->covAtLastSite(bz).Determinant();

    // Merge only the common current-surface continuation state, then keep one
    // real measurement history as the representative branch.
    if (comps[bi]->weight < comps[bj]->weight)
      std::swap(bi, bj);
    momentMerge(comps[bi], comps[bj], bz);

    if (logger) {
      const double km = comps[bi]->helixAtLastSite(bz).GetKappa();
      std::ostringstream os;
      os.setf(std::ios::scientific, std::ios::floatfield);
      os.precision(4);
      os << "      reducer merge[" << mergeStep << "] pair=(" << origI << "," << origJ
         << ") id=(" << idI << "," << idJ
         << ") symKL=" << bestDist
         << " w=(" << wi << "," << wj << ")"
         << " kappa=(" << ki << "," << kj << ")"
         << " det=(" << detI << "," << detJ << ")"
         << " -> keep=" << bi << " id=" << comps[bi]->debugId
         << " w=" << comps[bi]->weight
         << " kappa=" << km;
      logger(os.str());
    }

    delete comps[bj];
    comps.erase(comps.begin() + bj);
    mergeStep++;
  }
}

} // namespace GsfMixture
