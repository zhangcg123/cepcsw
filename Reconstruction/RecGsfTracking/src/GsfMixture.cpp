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

static TMatrixD stateMean5(const TKalTrackState& state) {
  TMatrixD mean(5, 1);
  for (int r = 0; r < 5; r++) mean(r, 0) = state(r, 0);
  return mean;
}

static TMatrixD stateCov5(const TKalTrackState& state) {
  TMatrixD cov(5, 5);
  const auto& kCov = state.GetCovMat();
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      cov(r, c) = kCov(r, c);
  return cov;
}

static void overwriteState5(TKalTrackState& state,
                            const TMatrixD& mean,
                            const TMatrixD& cov) {
  for (int r = 0; r < 5; r++) state(r, 0) = mean(r, 0);

  TKalMatrix kCov = state.GetCovMat();
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      kCov(r, c) = cov(r, c);
  state.SetCovMat(kCov);
}

static void momentMergeState(TKalTrackState& keepState,
                             const TKalTrackState& dropState,
                             double wk, double wd) {
  TMatrixD muK = stateMean5(keepState);
  TMatrixD muD = stateMean5(dropState);

  TMatrixD mergedMu = muK;
  mergedMu *= wk;
  TMatrixD weightedDropMu = muD;
  weightedDropMu *= wd;
  mergedMu += weightedDropMu;

  TMatrixD covK = stateCov5(keepState);
  TMatrixD covD = stateCov5(dropState);
  TMatrixD dK = muK - mergedMu;
  TMatrixD dD = muD - mergedMu;
  TMatrixD dKT(TMatrixD::kTransposed, dK);
  TMatrixD dDT(TMatrixD::kTransposed, dD);

  TMatrixD mergedCov = covK + dK * dKT;
  mergedCov *= wk;
  TMatrixD dropCovTerm = covD + dD * dDT;
  dropCovTerm *= wd;
  mergedCov += dropCovTerm;

  overwriteState5(keepState, mergedMu, mergedCov);
}

static void momentMergeSite(TKalTrackSite& keepSite,
                            const TKalTrackSite& dropSite,
                            double wk, double wd) {
  const int nStates = std::min(keepSite.GetEntries(), dropSite.GetEntries());
  for (int j = 0; j < nStates; j++) {
    auto* keepState = dynamic_cast<TKalTrackState*>(keepSite.At(j));
    auto* dropState = dynamic_cast<const TKalTrackState*>(dropSite.At(j));
    if (!keepState || !dropState) continue;
    momentMergeState(*keepState, *dropState, wk, wd);
  }
}

static void momentMerge(GsfComponent* keep, GsfComponent* drop, double /*bz*/) {
  const double totalWeight = keep->weight + drop->weight;
  if (totalWeight <= 0.0) return;

  const double wk = keep->weight / totalWeight;
  const double wd = drop->weight / totalWeight;
  const std::string keepHistory = keep->debugHistory;
  const std::string dropHistory = drop->debugHistory;
  const double mergedChi2 = wk * keep->fitChi2 + wd * drop->fitChi2;

  if (keep->kaltrack && drop->kaltrack) {
    const int nSites = std::min(keep->kaltrack->GetEntriesFast(),
                                drop->kaltrack->GetEntriesFast());
    for (int i = 0; i < nSites; i++) {
      auto* keepSite = dynamic_cast<TKalTrackSite*>(keep->kaltrack->At(i));
      auto* dropSite = dynamic_cast<const TKalTrackSite*>(drop->kaltrack->At(i));
      if (!keepSite || !dropSite) continue;
      momentMergeSite(*keepSite, *dropSite, wk, wd);
    }
  }

  keep->weight = totalWeight;
  keep->fitChi2 = mergedChi2;
  if (!dropHistory.empty()) {
    keep->debugHistory = "merge(" + keepHistory + " | " + dropHistory + ")";
  }
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
  reduce(comps, maxN, bz, {});
}

void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            const std::function<void(const std::string&)>& logger) {
  if (maxN < 1) maxN = 1;

  int mergeStep = 0;
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

    const int origI = bi;
    const int origJ = bj;
    const double wi = comps[bi]->weight;
    const double wj = comps[bj]->weight;
    const double ki = comps[bi]->helixAtLastSite(bz).GetKappa();
    const double kj = comps[bj]->helixAtLastSite(bz).GetKappa();
    const double detI = comps[bi]->covAtLastSite().Determinant();
    const double detJ = comps[bj]->covAtLastSite().Determinant();

    // Merge by moment matching the common branch history, then keep the
    // merged trajectory as the representative component for propagation.
    if (comps[bi]->weight < comps[bj]->weight)
      std::swap(bi, bj);
    momentMerge(comps[bi], comps[bj], bz);

    if (logger) {
      const double km = comps[bi]->helixAtLastSite(bz).GetKappa();
      std::ostringstream os;
      os.setf(std::ios::scientific, std::ios::floatfield);
      os.precision(4);
      os << "      reducer merge[" << mergeStep << "] pair=(" << origI << "," << origJ
         << ") symKL=" << bestDist
         << " w=(" << wi << "," << wj << ")"
         << " kappa=(" << ki << "," << kj << ")"
         << " det=(" << detI << "," << detJ << ")"
         << " -> keep=" << bi << " w=" << comps[bi]->weight
         << " kappa=" << km;
      logger(os.str());
    }

    delete comps[bj];
    comps.erase(comps.begin() + bj);
    mergeStep++;
  }
}

// ============================================================================
void reduceTopN(std::vector<GsfComponent*>& comps, int maxN) {
  reduceTopN(comps, maxN, {});
}

void reduceTopN(std::vector<GsfComponent*>& comps, int maxN,
                const std::function<void(const std::string&)>& logger) {
  if (maxN < 1) maxN = 1;
  if ((int)comps.size() <= maxN) {
    normalizeWeights(comps);
    return;
  }

  normalizeWeights(comps);
  std::sort(comps.begin(), comps.end(),
            [](const GsfComponent* a, const GsfComponent* b) {
              return a->weight > b->weight;
            });

  if (logger) {
    std::ostringstream os;
    os.setf(std::ios::scientific, std::ios::floatfield);
    os.precision(4);
    os << "      reducer topN keep=" << maxN << " from=" << comps.size();
    logger(os.str());
    for (size_t i = 0; i < comps.size(); i++) {
      const double kappa = comps[i]->helixAtLastSite(0.0).GetKappa();
      std::ostringstream line;
      line.setf(std::ios::scientific, std::ios::floatfield);
      line.precision(4);
      line << "        topN[" << i << "] "
           << (i < (size_t)maxN ? "keep" : "drop")
           << " id=" << comps[i]->debugId
           << " w=" << comps[i]->weight
           << " kappa=" << kappa;
      logger(line.str());
    }
  }

  for (size_t i = maxN; i < comps.size(); i++) {
    delete comps[i];
  }
  comps.resize(maxN);
  normalizeWeights(comps);
}

} // namespace GsfMixture
