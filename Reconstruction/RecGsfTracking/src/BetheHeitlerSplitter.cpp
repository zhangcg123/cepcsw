#include "BetheHeitlerSplitter.h"
#include "GsfComponent.h"

#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <string>

/// The ACTS AtlasBetheHeitlerApprox<6,5> data, reproduced here to avoid
/// requiring Eigen/Boost transitive dependencies.
namespace {

struct BHComponent { double weight, mean, var; };

/// ACTS/ATLAS coefficient convention: c[0] is the highest-order term and
/// c[degree] is the constant term. This matches ACTS's Horner evaluation.
inline double poly(double x, const double* c, int degree) {
  double sum = 0;
  for (int i = 0; i <= degree; i++) sum = x * sum + c[i];
  return sum;
}

/// Current ACTS default parameterization from
/// Acts::makeDefaultBetheHeitlerApprox (BetheHeitler_cdf_nC6_O5.par).
/// All three quantities use the transformed convention.
static const double actsData[6][3][6] = {
  {{ 3.74397e4, -1.95241e4,  3.51047e3, -2.54377e2,  1.81080e1, -3.57643},
   { 3.56728e4, -1.78603e4,  2.81521e3, -8.93555e1, -1.14015e1,  0.255769},
   { 3.73938e4, -1.92800e4,  3.21580e3, -1.46203e2, -5.65392,  -2.78008}},
  {{-4.14035e4,  2.31883e4, -4.37145e3,  2.44289e2,  1.13098e1, -3.21230},
   {-2.06936e3,  2.65334e3, -1.01413e3,  1.78338e2, -1.85556e1,  1.91430},
   {-5.19068e4,  2.55327e4, -4.22147e3,  1.90227e2,  9.34602,  -4.80961}},
  {{ 2.52200e3, -4.86348e3,  2.11942e3, -3.84534e2,  2.94503e1, -2.83310},
   { 1.80405e3, -1.93347e3,  6.27196e2, -4.32429e1, -1.43533e1,  3.58782},
   {-4.61617e4,  1.78221e4, -1.95746e3, -8.80646e1,  3.43153e1, -7.57830}},
  {{ 4.94537e3, -2.08737e3,  1.78089e2,  2.29879e1, -5.52783,  -1.86800},
   { 4.60220e3, -1.62269e3, -1.57552e2,  2.01796e2, -5.01636e1,  6.47438},
   {-9.50373e4,  4.05517e4, -5.62596e3,  4.58534e1,  6.70479e1, -1.22430e1}},
  {{-1.04129e3,  1.15222e2, -2.70356e1,  3.18611e1, -7.78800,  -1.50242},
   {-2.71361e4,  2.00625e4, -6.19444e3,  1.10061e3, -1.29354e2,  1.08289e1},
   { 3.15252e4, -3.31508e4,  1.20371e4, -2.23822e3,  2.44396e2, -2.09130e1}},
  {{ 1.27751e4, -6.79813e3,  1.24650e3, -8.20622e1, -2.33476,   0.246459},
   { 3.64336e5, -2.08457e5,  4.33028e4, -3.67825e3,  4.22914e1,  1.42701e1},
   {-1.79298e6,  1.01843e6, -2.10037e5,  1.82222e4, -4.33573e2, -2.72725e1}}
};

/// Inverse logit transform: y = 1/(1+exp(-x))
inline double invLogit(double x) { return 1.0 / (1.0 + std::exp(-x)); }

constexpr double kActsNoChangeLimit = 0.0001;
constexpr double kActsSingleGaussianLimit = 0.002;
constexpr double kActsHigherLimit = 0.2;

constexpr double kCepcWeightFloor = 1e-12;
constexpr double kCepcMeanEpsilon = 1e-9;
constexpr double kCepcVarianceFloor = 1e-12;

// Analysis artifacts and their exact compiled representations live together
// under data/. Runtime does not parse JSON; these tables are included so the
// selected mixture remains deterministic and dependency-free.
#include "../data/CEPCRuntimeCategoryAligned9Clear/compiled_table.inc"

inline double boundedLogit(double value) {
  value = std::min(1.0 - kCepcMeanEpsilon,
                   std::max(kCepcMeanEpsilon, value));
  return std::log(value / (1.0 - value));
}

inline double stableInvLogit(double value) {
  if (value >= 0.0) {
    const double e = std::exp(-value);
    return 1.0 / (1.0 + e);
  }
  const double e = std::exp(value);
  return e / (1.0 + e);
}

template <size_t K, size_t N>
std::vector<BHComponent> cepcIntervalMixture(
    double x, const std::array<double, K>& knots,
    const double (&weights)[K][N], const double (&means)[K][N],
    const double (&variances)[K][N]) {
  std::vector<BHComponent> result(N);
  if (!(x > 0.0)) {
    result[0] = {1.0, 1.0, kCepcVarianceFloor};
    for (size_t i = 1; i < result.size(); ++i)
      result[i] = {0.0, 1.0, kCepcVarianceFloor};
    return result;
  }

  if (x < knots.front()) {
    const double fraction = x / knots.front();
    double weightSum = 0.0;
    for (size_t i = 0; i < result.size(); ++i) {
      const double zeroWeight = (i == 0) ? 1.0 : 0.0;
      result[i].weight = (1.0 - fraction) * zeroWeight +
                         fraction * weights[0][i];
      result[i].mean = 1.0 - fraction * (1.0 - means[0][i]);
      result[i].var = kCepcVarianceFloor * std::exp(
          fraction * std::log(variances[0][i] / kCepcVarianceFloor));
      weightSum += result[i].weight;
    }
    for (auto& component : result) component.weight /= weightSum;
    return result;
  }

  size_t lower = K - 1;
  size_t upper = lower;
  double fraction = 0.0;
  if (x < knots.back()) {
    upper = 1;
    while (upper < K && knots[upper] < x) ++upper;
    lower = upper - 1;
    fraction = ((std::log(x) - std::log(knots[lower])) /
                (std::log(knots[upper]) - std::log(knots[lower])));
  }

  if (lower == upper) {
    for (size_t i = 0; i < result.size(); ++i)
      result[i] = {weights[lower][i], means[lower][i], variances[lower][i]};
    return result;
  }

  std::array<double, N> unnormalized{};
  unnormalized[0] = 1.0;
  double weightSum = 1.0;
  for (size_t i = 1; i < result.size(); ++i) {
    const double leftCoordinate = std::log(
        std::max(weights[lower][i], kCepcWeightFloor) /
        std::max(weights[lower][0], kCepcWeightFloor));
    const double rightCoordinate = std::log(
        std::max(weights[upper][i], kCepcWeightFloor) /
        std::max(weights[upper][0], kCepcWeightFloor));
    unnormalized[i] = std::exp((1.0 - fraction) * leftCoordinate +
                               fraction * rightCoordinate);
    weightSum += unnormalized[i];
  }
  for (size_t i = 0; i < result.size(); ++i) {
    result[i].weight = unnormalized[i] / weightSum;
    if (i == 0) {
      result[i].mean = 1.0;
      result[i].var = kCepcVarianceFloor;
    } else {
      result[i].mean = stableInvLogit(
          (1.0 - fraction) * boundedLogit(means[lower][i]) +
          fraction * boundedLogit(means[upper][i]));
      result[i].var = std::exp(
          (1.0 - fraction) * std::log(variances[lower][i]) +
          fraction * std::log(variances[upper][i]));
    }
  }
  return result;
}

std::vector<BHComponent> cepcRuntimeCategoryAligned9ClearMixture(double x) {
  return cepcIntervalMixture(
      x, RuntimeCategoryAligned9ClearTX0,
      RuntimeCategoryAligned9ClearWeights,
      RuntimeCategoryAligned9ClearMeans,
      RuntimeCategoryAligned9ClearVariances);
}

/// Build the 6-component Bethe-Heitler mixture for path length x (in X0).
/// Returns up to 6 (weight, mean, var) tuples.
std::vector<BHComponent> actsAtlasMixture(double x) {
  std::vector<BHComponent> result(6);

  if (x < kActsNoChangeLimit) {
    // negligible material: no energy loss
    result.resize(1);
    result[0] = {1.0, 1.0, 0.0};
    return result;
  }
  if (x < kActsSingleGaussianLimit) {
    // Exact first two moments of the Bethe-Heitler retained-energy fraction.
    // With c=x/log(2), E[z^n]=(n+1)^(-c).
    result.resize(1);
    const double mean = std::exp(-x);
    const double secondMoment =
        std::exp(-x * std::log(3.0) / std::log(2.0));
    result[0] = {1.0, mean, std::max(0.0, secondMoment - mean * mean)};
    return result;
  }

  // Current ACTS default: one transformed parameterization over [0, 0.2],
  // evaluated with highest-order-first Horner coefficients. ACTS treats 0.2
  // as the fallback evaluation point if the caller supplies a larger value.
  double xx = std::min(x, kActsHigherLimit);
  double weightSum = 0;
  for (int i = 0; i < 6; i++) {
    result[i].weight = invLogit(poly(xx, actsData[i][0], 5));
    result[i].mean   = invLogit(poly(xx, actsData[i][1], 5));
    result[i].var    = std::exp(poly(xx, actsData[i][2], 5));
    weightSum += result[i].weight;
  }
  for (int i = 0; i < 6; i++) result[i].weight /= weightSum;
  return result;
}

} // anonymous namespace

// ============================================================================

BetheHeitlerSplitter::BetheHeitlerSplitter() = default;

BetheHeitlerSplitter::BetheHeitlerSplitter(Model model)
    : m_model(model) {}

BetheHeitlerSplitter::BetheHeitlerSplitter(const std::string& modelName)
    : m_model(modelFromName(modelName)) {}

BetheHeitlerSplitter::Model BetheHeitlerSplitter::modelFromName(const std::string& modelName) {
  if (modelName == "ActsAtlas" || modelName == "actsAtlas" ||
      modelName == "ACTS" || modelName == "Acts") {
    return Model::ActsAtlas;
  }
  if (modelName == "CEPCRuntimeCategoryAligned9Clear") {
    return Model::CEPCRuntimeCategoryAligned9Clear;
  }
  throw std::invalid_argument("Unknown Bethe-Heitler model option: " + modelName);
}

const char* BetheHeitlerSplitter::modelName(Model model) {
  switch (model) {
    case Model::ActsAtlas: return "ActsAtlas";
    case Model::CEPCRuntimeCategoryAligned9Clear:
      return "CEPCRuntimeCategoryAligned9Clear";
  }
  return "Unknown";
}

namespace {

std::vector<GsfComponent*> applyMixture(
    GsfComponent* parent, const std::vector<BHComponent>& mixture, double bz,
    bool reverse,
    std::vector<BetheHeitlerMixtureComponent>* returnedMixture) {
  const double parentKappa = parent->helixAtLastSite(bz).GetKappa();
  const double parentWeight = parent->weight;
  const bool parentNoRadiationLineage = parent->noRadiationLineage;
  if (!parent->continuationValid && !parent->snapshotContinuation(bz)) {
    if (returnedMixture) returnedMixture->clear();
    return {parent};
  }
  if (returnedMixture) {
    returnedMixture->clear();
    returnedMixture->reserve(mixture.size());
    for (const auto& component : mixture) {
      returnedMixture->push_back(
          {component.weight, component.mean, component.var});
    }
  }
  std::vector<GsfComponent*> result;
  result.reserve(mixture.size());
  for (size_t i = 0; i < mixture.size(); i++) {
    result.push_back((i == 0) ? parent : parent->clone());
  }

  for (size_t i = 0; i < mixture.size(); i++) {
    double fracMomentum = std::max(mixture[i].mean, 0.01);
    double newKappa = reverse ? parentKappa * fracMomentum
                              : parentKappa / fracMomentum;

    GsfComponent* child = result[i];
    child->weight = parentWeight * mixture[i].weight;
    const bool exactIdentity =
        std::abs(mixture[i].mean - 1.0) <= 1e-15 &&
        mixture[i].var <= kCepcVarianceFloor;
    child->noRadiationLineage =
        parentNoRadiationLineage && exactIdentity;
    {
      std::ostringstream dbg;
      dbg.setf(std::ios::fixed, std::ios::floatfield);
      dbg.precision(4);
      dbg << "g" << i
          << "[w=" << mixture[i].weight
          << ",f=" << mixture[i].mean
          << ",s=" << std::sqrt(std::max(mixture[i].var, 0.0)) << "]";
      if (!child->debugHistory.empty()) child->debugHistory += "->";
      child->debugHistory += dbg.str();
      constexpr std::size_t maxHistoryLength = 4096;
      constexpr std::size_t historyEdgeLength = 2000;
      if (child->debugHistory.size() > maxHistoryLength) {
        child->debugHistory =
            child->debugHistory.substr(0, historyEdgeLength) +
            "...<history-truncated>..." +
            child->debugHistory.substr(
                child->debugHistory.size() - historyEdgeLength);
      }
    }

    // Preserve the filtered measurement state in the Kalman history.  The BH
    // process changes only the surface-local continuation snapshot used to
    // initialize propagation toward the next measurement.
    auto& continuation = child->continuationState;
    const double scaleKappa = reverse ? fracMomentum : 1.0 / fracMomentum;
    if (child->pendingProcessJacobian.GetNrows() != 5 ||
        child->pendingProcessJacobian.GetNcols() != 5) {
      child->pendingProcessJacobian.ResizeTo(5, 5);
      child->pendingProcessJacobian.UnitMatrix();
    }
    child->pendingProcessJacobian(2, 2) *= scaleKappa;
    const double invFrac = 1.0 / fracMomentum;
    const double invFrac2 = invFrac * invFrac;
    const double fracVar = std::max(mixture[i].var, 0.0);
    const double bhKappaVar = reverse
        ? parentKappa * parentKappa * fracVar
        : parentKappa * parentKappa * fracVar * invFrac2 * invFrac2;
    const double alpha = bz * 2.99792458e-4;
    auto covIndex = [](int row, int col) {
      if (row < col) std::swap(row, col);
      return row * (row + 1) / 2 + col;
    };
    for (int r = 0; r < 5; ++r)
      continuation.covMatrix[covIndex(r, 2)] *= scaleKappa;
    continuation.covMatrix[covIndex(2, 2)] *= scaleKappa;
    continuation.covMatrix[covIndex(2, 2)] += alpha * alpha * bhKappaVar;
    continuation.omega = newKappa * alpha;
    child->continuationValid = true;
  }

  return result;
}

}  // namespace

std::vector<BetheHeitlerMixtureComponent>
BetheHeitlerSplitter::mixture(double tX0) const {
  std::vector<BHComponent> internal;
  switch (m_model) {
    case Model::ActsAtlas:
      internal = actsAtlasMixture(tX0);
      break;
    case Model::CEPCRuntimeCategoryAligned9Clear:
      internal = cepcRuntimeCategoryAligned9ClearMixture(tX0);
      break;
  }
  std::vector<BetheHeitlerMixtureComponent> result;
  result.reserve(internal.size());
  for (const auto& component : internal)
    result.push_back({component.weight, component.mean, component.var});
  return result;
}

std::vector<GsfComponent*> BetheHeitlerSplitter::split(
    GsfComponent* parent, double tX0, double bz, bool reverse,
    std::vector<BetheHeitlerMixtureComponent>* returnedMixture) const {
  const auto publicMixture = mixture(tX0);
  std::vector<BHComponent> internal;
  internal.reserve(publicMixture.size());
  for (const auto& component : publicMixture)
    internal.push_back({component.weight, component.mean,
                        component.variance});
  return applyMixture(parent, internal, bz, reverse, returnedMixture);
}

std::vector<GsfComponent*> BetheHeitlerSplitter::splitWithRetainedFraction(
    GsfComponent* parent, double retainedFraction, double bz, bool reverse,
    std::vector<BetheHeitlerMixtureComponent>* returnedMixture) const {
  if (!std::isfinite(retainedFraction) || retainedFraction <= 0.0 ||
      retainedFraction > 1.0) {
    throw std::invalid_argument(
        "Truth BH retained-momentum fraction must be finite and in (0, 1]");
  }
  const std::vector<BHComponent> mixture = {
      {1.0, retainedFraction, kCepcVarianceFloor}};
  return applyMixture(parent, mixture, bz, reverse, returnedMixture);
}
