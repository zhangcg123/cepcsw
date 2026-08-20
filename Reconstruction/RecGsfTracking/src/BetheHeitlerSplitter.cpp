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

/// Same-sample execution artifact from
/// data/CEPC2GeV85StepConditioned/cepc2gev85_step_conditioned.json.
/// Interpolation follows that artifact exactly; this scoped model is not
/// physics validation.
constexpr size_t kCepcKnotCount = 8;
constexpr size_t kCepcComponentCount = 5;
constexpr size_t kCepc6ComponentCount = 6;
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

/// Same Geant4 transition sample and t/X0 knots as the five-component model,
/// with its g2/g3 loss range represented by three fixed truth strata:
/// 1--5%, 5--10%, and 10--20%. The total probability of that range is
/// unchanged at every knot.
static constexpr double cepc6Weights[kCepcKnotCount][kCepc6ComponentCount] = {
    {0.9992161956006349, 0.00053435586616364648, 0.00010720102253283032, 3.7520357886490611e-05, 3.7520357886490611e-05, 6.7206794895582076e-05},
    {0.99698032339762332, 0.0020212351451392952, 0.00046269238262224824, 7.3056691992986567e-05, 0.0002191700759789597, 0.00024352230664328856},
    {0.99255970834056706, 0.0040673594404900685, 0.0012896505543017289, 0.00064482527715086445, 0.00034721361077354239, 0.0010912427767168476},
    {0.95517035265989236, 0.024905359633393107, 0.0068738792588164972, 0.0025901574018728831, 0.0034867503486750349, 0.0069735006973500697},
    {0.91055139980431143, 0.051384022977622068, 0.01537101915853928, 0.0064072215383644228, 0.0054919041757409337, 0.010794432345421835},
    {0.85261546772305763, 0.079049248221842089, 0.025704510668947515, 0.011254164040695059, 0.010713964166741696, 0.020662645178716129},
    {0.81289838770153711, 0.1057367829021372, 0.032620922384701906, 0.01012373453318335, 0.011623547056617922, 0.026996625421822264},
    {0.74696707105719251, 0.14904679376083191, 0.041594454072790304, 0.013864818024263433, 0.0051993067590987881, 0.043327556325823233}};

static constexpr double cepc6Means[kCepcKnotCount][kCepc6ComponentCount] = {
    {1, 0.99884766922776635, 0.97556244551142235, 0.92451213355707562, 0.85820475235480187, 0.53608743511241941},
    {1, 0.99864807156942648, 0.9787054050959042, 0.92816179326230586, 0.86024706215365176, 0.61697072883970228},
    {1, 0.99832926825823465, 0.97494882917550085, 0.92662383325981112, 0.85627094566474038, 0.58098416102467987},
    {1, 0.99851495232738763, 0.97443366950445343, 0.93155296783396557, 0.8601911260287789, 0.53856583951698356},
    {1, 0.99822985785928486, 0.97589057895364695, 0.92808573657958204, 0.85887270968814478, 0.52783716531564129},
    {1, 0.99792054624476967, 0.97467254405723969, 0.92739087540549381, 0.85825377378790413, 0.54757216213392146},
    {1, 0.99779899962903329, 0.97689828365181486, 0.92554798108943137, 0.85803519272978823, 0.4773879349825132},
    {1, 0.99750486973143293, 0.97990463695279117, 0.9306686402578711, 0.86490567734182822, 0.54602374590582048}};

static constexpr double cepc6Variances[kCepcKnotCount][kCepc6ComponentCount] = {
    {1e-12, 4.262027532009327e-06, 0.00012485967283637489, 0.00022920596108721991, 0.00086949856754225952, 0.048919630665790581},
    {1e-12, 4.6506375428467805e-06, 7.2068322021001663e-05, 5.9121084847735261e-05, 0.00060494972363256405, 0.018047767691370842},
    {1e-12, 5.7493933481866932e-06, 0.00013542442016811762, 0.00021709794041835373, 0.00035790601940688394, 0.04868662435589749},
    {1e-12, 4.3696375864321624e-06, 0.00011800267935146991, 0.00020042781311735425, 0.00073986169396511592, 0.048712889751143851},
    {1e-12, 5.3654404840175474e-06, 0.00012498784475767355, 0.0001822534224584782, 0.00084663497922043973, 0.04706965545977787},
    {1e-12, 6.3959788880740831e-06, 0.00012434290648866142, 0.00022810280888230228, 0.00076771182119095283, 0.041794329167051114},
    {1e-12, 6.5596673156642638e-06, 0.00011223656930814396, 0.00018876637340781155, 0.00081059249911230591, 0.065828372132041429},
    {1e-12, 5.4917847350788307e-06, 4.9641302950043098e-05, 0.00020993584933803877, 8.2928655291025777e-05, 0.058511627454352233}};

// Analysis artifacts and their exact compiled representations live together
// under data/. Runtime does not parse JSON; these tables are included so the
// selected mixture remains deterministic and dependency-free.
#include "../data/CEPCRuntimeGenericGrid5Clear/compiled_table.inc"
#include "../data/CEPCRuntimeCategoryAligned5Clear/compiled_table.inc"

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
std::vector<BHComponent> cepcStepConditionedMixture(
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

std::vector<BHComponent> cepc2GeV85StepConditionedMixture(double x) {
  return cepcStepConditionedMixture(
      x, cepcTX0, cepcWeights, cepcMeans, cepcVariances);
}

std::vector<BHComponent> cepc2GeV85StepConditioned6Mixture(double x) {
  return cepcStepConditionedMixture(
      x, cepcTX0, cepc6Weights, cepc6Means, cepc6Variances);
}

std::vector<BHComponent> cepcRuntimeGenericGrid5ClearMixture(double x) {
  return cepcStepConditionedMixture(
      x, RuntimeGenericGrid5ClearTX0, RuntimeGenericGrid5ClearWeights,
      RuntimeGenericGrid5ClearMeans, RuntimeGenericGrid5ClearVariances);
}

std::vector<BHComponent> cepcRuntimeCategoryAligned5ClearMixture(double x) {
  return cepcStepConditionedMixture(
      x, RuntimeCategoryAligned5ClearTX0,
      RuntimeCategoryAligned5ClearWeights,
      RuntimeCategoryAligned5ClearMeans,
      RuntimeCategoryAligned5ClearVariances);
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
  if (modelName == "CEPC2GeV85StepConditioned" ||
      modelName == "cepc2GeV85StepConditioned") {
    return Model::CEPC2GeV85StepConditioned;
  }
  if (modelName == "CEPC2GeV85StepConditioned6" ||
      modelName == "cepc2GeV85StepConditioned6") {
    return Model::CEPC2GeV85StepConditioned6;
  }
  if (modelName == "CEPCRuntimeGenericGrid5Clear") {
    return Model::CEPCRuntimeGenericGrid5Clear;
  }
  if (modelName == "CEPCRuntimeCategoryAligned5Clear") {
    return Model::CEPCRuntimeCategoryAligned5Clear;
  }
  throw std::invalid_argument("Unknown Bethe-Heitler model option: " + modelName);
}

const char* BetheHeitlerSplitter::modelName(Model model) {
  switch (model) {
    case Model::ActsAtlas: return "ActsAtlas";
    case Model::CEPC2GeV85StepConditioned:
      return "CEPC2GeV85StepConditioned";
    case Model::CEPC2GeV85StepConditioned6:
      return "CEPC2GeV85StepConditioned6";
    case Model::CEPCRuntimeGenericGrid5Clear:
      return "CEPCRuntimeGenericGrid5Clear";
    case Model::CEPCRuntimeCategoryAligned5Clear:
      return "CEPCRuntimeCategoryAligned5Clear";
  }
  return "Unknown";
}

std::vector<GsfComponent*> BetheHeitlerSplitter::split(
    GsfComponent* parent, double tX0, double bz, bool reverse,
    std::vector<BetheHeitlerMixtureComponent>* returnedMixture) const {

  std::vector<BHComponent> mixture;
  switch (m_model) {
    case Model::ActsAtlas:
      mixture = actsAtlasMixture(tX0);
      break;
    case Model::CEPC2GeV85StepConditioned:
      mixture = cepc2GeV85StepConditionedMixture(tX0);
      break;
    case Model::CEPC2GeV85StepConditioned6:
      mixture = cepc2GeV85StepConditioned6Mixture(tX0);
      break;
    case Model::CEPCRuntimeGenericGrid5Clear:
      mixture = cepcRuntimeGenericGrid5ClearMixture(tX0);
      break;
    case Model::CEPCRuntimeCategoryAligned5Clear:
      mixture = cepcRuntimeCategoryAligned5ClearMixture(tX0);
      break;
  }
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
