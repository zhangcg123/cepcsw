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

/// Low-x/x0 parameterization (6 components, 5th-degree poly, transformed)
static const double lowData[6][3][6] = {
  {{-2.602291, 7.77102, -17.9787, 18.1642, -3.84789, -0.619996},
   {-0.97768, 7.66173, -15.1885, 14.2331, -2.82253, -0.499998},
   { 0.00212652, 0.0989093, -0.401838, -1.35065, 4.16609, -2.51854}},
  {{-1.53086, 3.42102, 3.11248, -9.55927, 5.60646, -0.126554},
   {-7.72904, 80.7146, -300.495, 494.036, -348.391, 79.1706},
   { 2.01418, -37.1566, 257.318, -830.857, 1279.6, -764.537}},
  {{-2.64613, 0.333018, 37.0482, -78.5174, 55.6362, -13.0697},
   {-5.08739, 55.3222, -213.947, 377.555, -307.269, 93.1098},
   {-4.00144, 29.8452, -92.0532, 156.158, -146.11, 59.8256}},
  {{-3.46268, -1.47009, 24.9061, -51.3185, 35.1081, -7.97863},
   {-5.34997, 57.1566, -219.745, 387.278, -315.543, 95.6884},
   {-4.80241, 32.8814, -104.929, 184.326, -178.488, 75.3148}},
  {{-4.81887, 14.5956, -22.3837, 7.83819, 5.82193, -3.09581},
   {-5.45581, 57.8864, -222.817, 392.928, -320.46, 97.3029},
   {-5.00944, 32.4538, -108.483, 192.421, -187.429, 79.2258}},
  {{-2.5476, 8.14925, -7.61825, 1.18039, 1.75554, -0.874916},
   {-5.21871, 56.3462, -216.399, 381.145, -310.829, 94.4863},
   {-5.83194, 31.6169, -109.537, 196.603, -193.167, 81.9951}}
};

/// High-x/x0 parameterization (6 components, 5th-degree poly, untransformed)
static const double highData[6][3][6] = {
  {{ 0.116785, 0.00300851, -0.00500615, 0.0162373, -0.0147852, 0.00507151},
   { 0.929508, 0.0968065, -0.6902, 0.924948, -0.511764, 0.106812},
   { 0.000895549, 0.00345136, -0.0319301, 0.0696704, -0.0659913, 0.0236099}},
  {{ 0.389022, -2.44128, 5.36186, -5.71814, 2.93331, -0.585089},
   { 0.32526, 2.93045, -9.33945, 12.6618, -7.81376, 1.82936},
   { 0.000439302, 0.0173778, 0.113849, -0.584371, 0.758494, -0.327375}},
  {{-0.0135153, 6.61798, -17.0068, 20.0236, -11.1439, 2.38296},
   {-0.554228, 8.67827, -23.9133, 30.2601, -17.6788, 3.95462},
   {-0.00209441, -0.0577078, -0.55689, 1.76433, -1.78493, 0.649452}},
  {{ 0.0614916, 4.70061, -12.7008, 15.3499, -8.68459, 1.87726},
   {-0.651238, 9.61423, -25.5828, 31.5933, -18.1433, 4},
   { 6.4614e-5, 0.0279685, -0.168134, 0.52802, -0.522842, 0.192059}},
  {{ 0.0356266, 5.44894, -14.28, 16.9637, -9.46193, 2.02694},
   {-0.701619, 10.1539, -26.5785, 32.3834, -18.3943, 4.02389},
   { 0.0044289, -0.0797969, 0.188953, -0.16965, 0.049072, 0.0024294}},
  {{-0.496513, 19.0152, -79.0763, 147.496, -128.503, 42.6193},
   {-1.1283, 14.6583, -35.5871, 40.9352, -22.1949, 4.67131},
   { 0.00387964, -0.00746174, 0.0129534, -0.00999381, -0.00134676, 0.00495766}}
};

/// Inverse logit transform: y = 1/(1+exp(-x))
inline double invLogit(double x) { return 1.0 / (1.0 + std::exp(-x)); }

constexpr double kActsNoChangeLimit = 0.0001;
constexpr double kActsSingleGaussianLimit = 0.002;
constexpr double kActsLowerParameterLimit = 0.1;
constexpr double kActsHigherLimit = 0.2;

/// Same-sample execution artifact from
/// data/CEPC2GeV85StepConditioned/cepc2gev85_step_conditioned.json.
/// Interpolation follows that artifact exactly; this scoped model is not
/// physics validation.
constexpr size_t kCepcKnotCount = 8;
constexpr size_t kCepcComponentCount = 5;
constexpr double kCepcWeightFloor = 1e-12;
constexpr double kCepcMeanEpsilon = 1e-9;
constexpr double kCepcVarianceFloor = 1e-12;

static constexpr std::array<double, kCepcKnotCount> cepcTX0 = {{
    5.0000000000000002e-05, 0.00022360679774997898,
    0.001, 0.0031622776601683794, 0.0070710678118654753,
    0.012247448713915889, 0.017320508075688773,
    0.024494897427831779}};

static constexpr double cepcWeights[kCepcKnotCount][kCepcComponentCount] = {
    {0.9992161956006349, 0.00053435586616364648, 0.00010720102253283032, 7.5040715772981223e-05, 6.7206794895582076e-05},
    {0.99698032339762332, 0.0020212351451392952, 0.00046269238262224824, 0.00029222676797194627, 0.00024352230664328856},
    {0.99255970834056695, 0.0040673594404900676, 0.0012896505543017287, 0.00099203888792440673, 0.0010912427767168474},
    {0.95517035265989236, 0.024905359633393107, 0.0068738792588164972, 0.0060769077505479175, 0.0069735006973500697},
    {0.91055139980431143, 0.051384022977622068, 0.01537101915853928, 0.011899125714105356, 0.010794432345421835},
    {0.85261546772305752, 0.079049248221842075, 0.025704510668947512, 0.021968128207436752, 0.020662645178716126},
    {0.81289838770153711, 0.1057367829021372, 0.032620922384701906, 0.021747281589801271, 0.026996625421822264},
    {0.74696707105719251, 0.14904679376083191, 0.041594454072790304, 0.019064124783362221, 0.043327556325823233}};

static constexpr double cepcMeans[kCepcKnotCount][kCepcComponentCount] = {
    {1, 0.99884766922776635, 0.97556244551142235, 0.89135844295593858, 0.53608743511241941},
    {1, 0.99864807156942648, 0.9787054050959042, 0.8772257449308154, 0.61697072883970228},
    {1, 0.99832926825823465, 0.97494882917550085, 0.9020003226015364, 0.58098416102467987},
    {1, 0.99851495232738763, 0.97443366950445343, 0.89060764876541576, 0.53856583951698356},
    {1, 0.99822985785928486, 0.97589057895364695, 0.89614126262968785, 0.52783716531564129},
    {1, 0.99792054624476967, 0.97467254405723969, 0.89367237092806295, 0.54757216213392146},
    {1, 0.99779899962903329, 0.97689828365181486, 0.88946355972479463, 0.4773879349825132},
    {1, 0.99750486973143293, 0.97990463695279117, 0.91273328673531384, 0.54602374590582048}};

static constexpr double cepcVariances[kCepcKnotCount][kCepcComponentCount] = {
    {1e-12, 4.262027532009327e-06, 0.00012485967283637489, 0.0016485194647909429, 0.048919630665790581},
    {1e-12, 4.6506375428467805e-06, 7.2068322021001663e-05, 0.0013333195704788858, 0.018047767691370842},
    {1e-12, 5.7493933481866932e-06, 0.00013542442016811762, 0.0013923985684636264, 0.04868662435589749},
    {1e-12, 4.3696375864321624e-06, 0.00011800267935146991, 0.0017553532846341646, 0.048712889751143851},
    {1e-12, 5.3654404840175474e-06, 0.00012498784475767355, 0.0016794153826326097, 0.04706965545977787},
    {1e-12, 6.3959788880740831e-06, 0.00012434290648866142, 0.0016855349042544931, 0.041794329167051114},
    {1e-12, 6.5596673156642638e-06, 0.00011223656930814396, 0.0016551961539066351, 0.065828372132041429},
    {1e-12, 5.4917847350788307e-06, 4.9641302950043098e-05, 0.0010331026062999626, 0.058511627454352233}};

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

std::vector<BHComponent> cepc2GeV85StepConditionedMixture(double x) {
  std::vector<BHComponent> result(kCepcComponentCount);
  if (!(x > 0.0)) {
    result[0] = {1.0, 1.0, kCepcVarianceFloor};
    for (size_t i = 1; i < result.size(); ++i)
      result[i] = {0.0, 1.0, kCepcVarianceFloor};
    return result;
  }

  if (x < cepcTX0.front()) {
    const double fraction = x / cepcTX0.front();
    double weightSum = 0.0;
    for (size_t i = 0; i < result.size(); ++i) {
      const double zeroWeight = (i == 0) ? 1.0 : 0.0;
      result[i].weight = (1.0 - fraction) * zeroWeight +
                         fraction * cepcWeights[0][i];
      result[i].mean = 1.0 - fraction * (1.0 - cepcMeans[0][i]);
      result[i].var = kCepcVarianceFloor * std::exp(
          fraction * std::log(cepcVariances[0][i] / kCepcVarianceFloor));
      weightSum += result[i].weight;
    }
    for (auto& component : result) component.weight /= weightSum;
    return result;
  }

  size_t lower = kCepcKnotCount - 1;
  size_t upper = lower;
  double fraction = 0.0;
  if (x < cepcTX0.back()) {
    upper = 1;
    while (upper < kCepcKnotCount && cepcTX0[upper] < x) ++upper;
    lower = upper - 1;
    fraction = ((std::log(x) - std::log(cepcTX0[lower])) /
                (std::log(cepcTX0[upper]) - std::log(cepcTX0[lower])));
  }

  if (lower == upper) {
    for (size_t i = 0; i < result.size(); ++i)
      result[i] = {cepcWeights[lower][i], cepcMeans[lower][i],
                   cepcVariances[lower][i]};
    return result;
  }

  std::array<double, kCepcComponentCount> unnormalized{};
  unnormalized[0] = 1.0;
  double weightSum = 1.0;
  for (size_t i = 1; i < result.size(); ++i) {
    const double leftCoordinate = std::log(
        std::max(cepcWeights[lower][i], kCepcWeightFloor) /
        std::max(cepcWeights[lower][0], kCepcWeightFloor));
    const double rightCoordinate = std::log(
        std::max(cepcWeights[upper][i], kCepcWeightFloor) /
        std::max(cepcWeights[upper][0], kCepcWeightFloor));
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
          (1.0 - fraction) * boundedLogit(cepcMeans[lower][i]) +
          fraction * boundedLogit(cepcMeans[upper][i]));
      result[i].var = std::exp(
          (1.0 - fraction) * std::log(cepcVariances[lower][i]) +
          fraction * std::log(cepcVariances[upper][i]));
    }
  }
  return result;
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

  if (x < kActsLowerParameterLimit) {
    // Low-x parameterization (transformed)
    double weightSum = 0;
    for (int i = 0; i < 6; i++) {
      result[i].weight = invLogit(poly(x, lowData[i][0], 5));
      result[i].mean   = invLogit(poly(x, lowData[i][1], 5));
      result[i].var    = std::exp(poly(x, lowData[i][2], 5));
      weightSum += result[i].weight;
    }
    for (int i = 0; i < 6; i++) result[i].weight /= weightSum;
    return result;
  }

  // High-x parameterization (untransformed, capped at 0.2)
  double xx = std::min(x, kActsHigherLimit);
  double weightSum = 0;
  for (int i = 0; i < 6; i++) {
    result[i].weight = poly(xx, highData[i][0], 5);
    result[i].mean   = poly(xx, highData[i][1], 5);
    result[i].var    = poly(xx, highData[i][2], 5);
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
  if (modelName == "CEPC2GeV85StepConditioned" ||
      modelName == "cepc2GeV85StepConditioned") {
    return Model::CEPC2GeV85StepConditioned;
  }
  throw std::invalid_argument("Unknown Bethe-Heitler model option: " + modelName);
}

const char* BetheHeitlerSplitter::modelName(Model model) {
  switch (model) {
    case Model::ActsAtlas: return "ActsAtlas";
    case Model::CEPC2GeV85StepConditioned:
      return "CEPC2GeV85StepConditioned";
  }
  return "Unknown";
}

std::vector<GsfComponent*> BetheHeitlerSplitter::split(
    GsfComponent* parent, double tX0, double bz, bool reverse) const {

  std::vector<BHComponent> mixture;
  switch (m_model) {
    case Model::ActsAtlas:
      mixture = actsAtlasMixture(tX0);
      break;
    case Model::CEPC2GeV85StepConditioned:
      mixture = cepc2GeV85StepConditionedMixture(tX0);
      break;
  }
  const double parentKappa = parent->helixAtLastSite(bz).GetKappa();
  const double parentWeight = parent->weight;
  const bool parentNoRadiationLineage = parent->noRadiationLineage;
  if (!parent->continuationValid && !parent->snapshotContinuation(bz)) {
    return {parent};
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
