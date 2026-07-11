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
    {0.9965097820933061, 0.0032185045457356672, 0.00012245655266250233, 7.9576143649370189e-05, 6.9680664646339698e-05},
    {0.87008084940580566, 0.12831190337034876, 0.0009984414572374832, 0.000340931229300604, 0.00026787453730761743},
    {0.95744153170804291, 0.038342303018278318, 0.0019840777758488135, 0.0010912427767168474, 0.0011408447211130677},
    {0.59932257421797175, 0.36600916517234511, 0.017931858936043037, 0.0094640366606893803, 0.0072723650129507874},
    {0.0018937600606003223, 0.93838967269513629, 0.031404854338288678, 0.016475712527222804, 0.011836000378752013},
    {0.0, 0.9149185198523454, 0.03722877464661925, 0.026244710542900875, 0.021607994958134511},
    {0.0, 0.84926884139482561, 0.070866141732283464, 0.043869516310461196, 0.0359955005624297},
    {0.0, 0.5597920277296361, 0.27036395147313691, 0.079722703639514725, 0.090121317157712308}};

static constexpr double cepcMeans[kCepcKnotCount][kCepcComponentCount] = {
    {0.99999811391315019, 0.99938688500060302, 0.97617963579030098, 0.89269820354133711, 0.54228065295083616},
    {0.99998047457474859, 0.99961208001603763, 0.98078234822899879, 0.88097879266387447, 0.61056764207947289},
    {0.99998509587401396, 0.99911725277295882, 0.97716525500206708, 0.90243815104595593, 0.57007756399784459},
    {0.99991481811354443, 0.99953613012669662, 0.97564721474571958, 0.89909651661696777, 0.54352176936338537},
    {0.99991999301828394, 0.99953248416091778, 0.97675773836350976, 0.89851360714856876, 0.53865488274246753},
    {1.0, 0.9993549086350817, 0.97524610025529535, 0.89423348465003205, 0.55164047060437005},
    {1.0, 0.99856506652807908, 0.976923320066443, 0.89589451425672062, 0.50750544512066587},
    {1.0, 0.99712776666391623, 0.97842076290488311, 0.9007924013632066, 0.52141167311299952}};

static constexpr double cepcVariances[kCepcKnotCount][kCepcComponentCount] = {
    {3.3372860031022356e-11, 1.4912932545518842e-06, 0.00012116083220237162, 0.0016667582707591277, 0.048297685187217387},
    {3.6621883303666891e-10, 4.1020433305671844e-07, 5.7236966114704302e-05, 0.0012285854352461767, 0.016812978691541913},
    {1.5192502811345321e-10, 2.8139546006666905e-06, 0.00012250065676688848, 0.0013636559662150161, 0.051250747922331974},
    {6.1595950562320922e-11, 1.4410967023037458e-06, 0.00012241265019885539, 0.0015480717521817455, 0.047326773871511407},
    {9.4471763745218595e-11, 1.063007537327465e-06, 0.00011859343832043567, 0.0016571173623998181, 0.045583739695020109},
    {1e-12, 1.6044829763695034e-06, 0.00012679998906051093, 0.0016828516931821635, 0.041075174846030127},
    {1e-12, 3.6069453316356359e-06, 0.00011915982398802427, 0.0015079659222358988, 0.056943939908073782},
    {1e-12, 7.2875506121894418e-06, 8.8171979417239754e-05, 0.0018923590562457404, 0.05778801862206423}};

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
    result[i].mean = stableInvLogit(
        (1.0 - fraction) * boundedLogit(cepcMeans[lower][i]) +
        fraction * boundedLogit(cepcMeans[upper][i]));
    result[i].var = std::exp(
        (1.0 - fraction) * std::log(cepcVariances[lower][i]) +
        fraction * std::log(cepcVariances[upper][i]));
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
