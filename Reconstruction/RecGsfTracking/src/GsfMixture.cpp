#include "GsfMixture.h"
#include "GsfComponent.h"

#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"

#include <cmath>
#include <algorithm>

namespace GsfMixture {

// ============================================================================
void normalizeWeights(std::vector<GsfComponent*>& comps) {
  double sum = 0.0;
  for (auto* c : comps) sum += c->weight;
  if (sum < 1e-30) {
    for (auto* c : comps) c->weight = 1.0 / comps.size();
    return;
  }
  for (auto* c : comps) c->weight /= sum;
}

// ============================================================================
/// Full-covariance KL divergence between two multivariate Gaussians.
///   KL(P‖Q) = ½ [ ln(|ΣQ|/|ΣP|) - d + tr(ΣQ⁻¹ ΣP) + (μQ-μP)ᵀ ΣQ⁻¹ (μQ-μP) ]
static double klDirectional(GsfComponent* p, GsfComponent* q,
                             const TMatrixD& muP, const TMatrixD& muQ) {
  TMatrixD covP = p->covAtLastSite();
  TMatrixD covQ = q->covAtLastSite();

  static constexpr int d = 5;
  constexpr double eps = 1e-12;

  // Determinants
  double detP = covP.Determinant();
  double detQ = covQ.Determinant();
  if (detP < eps || detQ < eps || std::isnan(detP) || std::isnan(detQ))
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

static void overwriteLastSiteStates(GsfComponent* comp,
                                    const TMatrixD& mean,
                                    const TMatrixD& cov) {
  if (!comp || !comp->kaltrack || comp->kaltrack->GetEntriesFast() == 0) return;
  auto* site = dynamic_cast<TKalTrackSite*>(comp->kaltrack->Last());
  if (!site) return;

  TKalMatrix kCov(5, 5);
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      kCov(r, c) = cov(r, c);

  for (int j = 0; j < site->GetEntries(); j++) {
    auto* st = dynamic_cast<TKalTrackState*>(site->At(j));
    if (!st) continue;
    for (int r = 0; r < 5; r++) (*st)(r, 0) = mean(r, 0);
    st->SetCovMat(kCov);
  }
}

static void momentMerge(GsfComponent* keep, GsfComponent* drop, double bz) {
  const double totalWeight = keep->weight + drop->weight;
  if (totalWeight <= 0.0) return;

  const double wk = keep->weight / totalWeight;
  const double wd = drop->weight / totalWeight;

  TMatrixD muK(5, 1), muD(5, 1);
  keep->helixAtLastSite(bz).PutInto(muK);
  drop->helixAtLastSite(bz).PutInto(muD);

  TMatrixD mergedMu = muK;
  mergedMu *= wk;
  TMatrixD weightedDropMu = muD;
  weightedDropMu *= wd;
  mergedMu += weightedDropMu;

  TMatrixD covK = keep->covAtLastSite();
  TMatrixD covD = drop->covAtLastSite();
  TMatrixD dK = muK - mergedMu;
  TMatrixD dD = muD - mergedMu;
  TMatrixD dKT(TMatrixD::kTransposed, dK);
  TMatrixD dDT(TMatrixD::kTransposed, dD);

  TMatrixD mergedCov = covK + dK * dKT;
  mergedCov *= wk;
  TMatrixD dropCovTerm = covD + dD * dDT;
  dropCovTerm *= wd;
  mergedCov += dropCovTerm;

  keep->weight = totalWeight;
  overwriteLastSiteStates(keep, mergedMu, mergedCov);
}

/// Symmetric KL distance: ½[KL(P‖Q) + KL(Q‖P)]
static double klDistance(GsfComponent* a, GsfComponent* b, double bz) {
  TMatrixD muA(5, 1), muB(5, 1);
  a->helixAtLastSite(bz).PutInto(muA);
  b->helixAtLastSite(bz).PutInto(muB);

  double klAB = klDirectional(a, b, muA, muB);
  double klBA = klDirectional(b, a, muB, muA);
  return 0.5 * (klAB + klBA);
}

// ============================================================================
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz) {
  if (maxN < 1) maxN = 1;

  while ((int)comps.size() > maxN) {
    int bi = -1, bj = -1;
    double bestDist = 1e30;
    for (size_t i = 0; i < comps.size(); i++) {
      for (size_t j = i + 1; j < comps.size(); j++) {
        double d = klDistance(comps[i], comps[j], bz);
        if (bi < 0 || d < bestDist) {
          bestDist = d;
          bi = (int)i;
          bj = (int)j;
        }
      }
    }
    if (bi < 0) break;

    // Merge by moment matching at the active site, then keep the merged
    // trajectory as the representative component for subsequent propagation.
    if (comps[bi]->weight < comps[bj]->weight)
      std::swap(bi, bj);
    momentMerge(comps[bi], comps[bj], bz);
    delete comps[bj];
    comps.erase(comps.begin() + bj);
  }
}

} // namespace GsfMixture
