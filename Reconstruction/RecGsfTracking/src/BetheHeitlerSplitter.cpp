#include "BetheHeitlerSplitter.h"
#include "GsfComponent.h"

#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <string>

/// The ACTS AtlasBetheHeitlerApprox<6,5> data, reproduced here to avoid
/// requiring Eigen/Boost transitive dependencies.
namespace {

struct BHComponent { double weight, mean, var; };

/// Evaluate polynomial: c[0] + c[1]*x + c[2]*x^2 + ... + c[degree]*x^degree
inline double poly(double x, const double* c, int degree) {
  double sum = 0;
  for (int i = degree; i >= 0; i--) sum = x * sum + c[i];
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

constexpr double kThinGaussianUpperX0 = 0.1;

/// Simulation-derived global retained-fraction model from
/// BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree.
///
/// Fit target: 2 GeV, theta=85 deg primary electron tracker eBrem,
/// z = post_p/pre_p.  Components are truncated Gaussians normalized on [0,1],
/// so the listed weights are already the in-range probability masses.  The
/// current split implementation uses weight and mean to create hypotheses;
/// the variance is retained here for model provenance/future covariance work.
std::vector<BHComponent> globalSim2GeV85Mixture(double /*x*/) {
  return {
      {0.077416116868, 0.677171066692, 0.35 * 0.35},
      {0.135334171174, 0.999993855825, 0.153904803245 * 0.153904803245},
      {0.125841439379, 0.999993855825, 0.0573680711812 * 0.0573680711812},
      {0.101560696051, 0.999993855825, 0.019581894445 * 0.019581894445},
      {0.559847576529, 0.999993855825, 0.00479768891507 * 0.00479768891507},
  };
}

/// Build the 6-component Bethe-Heitler mixture for path length x (in X0).
/// Returns up to 6 (weight, mean, var) tuples.
std::vector<BHComponent> bhMixture6(double x) {
  std::vector<BHComponent> result(6);

  if (x < 0.0001) {
    // negligible material: no energy loss
    result.resize(1);
    result[0] = {1.0, 1.0, 0.0};
    return result;
  }
  if (x < kThinGaussianUpperX0) {
    // CEPC thin-material toy mixture. Keep a dominant no-loss branch so
    // normal tracks are not forced to lose energy, plus one moderate-loss
    // tail branch. The weighted mean is constrained to exp(-x), matching
    // the thin-material Bethe-Heitler expectation E[p/p0].
    result.resize(2);
    double expectedMean = std::exp(-x);
    double tailWeight = std::min(0.20, std::max(0.02, 10.0 * x));
    double tailMean = (expectedMean - (1.0 - tailWeight)) / tailWeight;
    tailMean = std::min(0.999, std::max(0.50, tailMean));

    result[0] = {1.0 - tailWeight, 1.0, 0.0};
    result[1] = {tailWeight, tailMean, x * x};
    return result;
  }

  if (x < 0.1) {
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
  double xx = std::min(x, 0.2);
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
  if (modelName == "Current" || modelName == "current" || modelName == "default") {
    return Model::Current;
  }
  if (modelName == "GlobalSim2GeV85" || modelName == "globalSim2GeV85" ||
      modelName == "globalBHmodelfromSim@2GeV85Degree") {
    return Model::GlobalSim2GeV85;
  }
  throw std::invalid_argument("Unknown Bethe-Heitler model option: " + modelName);
}

const char* BetheHeitlerSplitter::modelName(Model model) {
  switch (model) {
    case Model::Current: return "Current";
    case Model::GlobalSim2GeV85: return "GlobalSim2GeV85";
  }
  return "Unknown";
}

std::vector<GsfComponent*> BetheHeitlerSplitter::split(
    GsfComponent* parent, double tX0, double bz) const {

  auto mixture = (m_model == Model::GlobalSim2GeV85)
      ? globalSim2GeV85Mixture(tX0)
      : bhMixture6(tX0);
  const double parentKappa = parent->helixAtLastSite(bz).GetKappa();
  const double parentWeight = parent->weight;
  std::vector<GsfComponent*> result;
  result.reserve(mixture.size());
  for (size_t i = 0; i < mixture.size(); i++) {
    result.push_back((i == 0) ? parent : parent->clone());
  }

  for (size_t i = 0; i < mixture.size(); i++) {
    double fracMomentum = std::max(mixture[i].mean, 0.01);
    double newKappa = parentKappa / fracMomentum;

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
    }

    if (child->kaltrack->GetEntriesFast() > 0) {
      auto* lastSite = dynamic_cast<TKalTrackSite*>(
          child->kaltrack->Last());
      if (lastSite) {
        const double invFrac = 1.0 / fracMomentum;
        const double invFrac2 = invFrac * invFrac;
        const double fracVar = std::max(mixture[i].var, 0.0);
        const double bhKappaVar = parentKappa * parentKappa * fracVar * invFrac2 * invFrac2;
        for (int j = 0; j < lastSite->GetEntries(); j++) {
          auto* st = dynamic_cast<TKalTrackState*>(lastSite->At(j));
          if (!st) continue;

          TKalMatrix cov = st->GetCovMat();
          for (int r = 0; r < cov.GetNrows(); r++) cov(r, 2) *= invFrac;
          for (int c = 0; c < cov.GetNcols(); c++) cov(2, c) *= invFrac;
          cov(2, 2) += bhKappaVar;

          (*st)(2, 0) = newKappa;
          st->SetCovMat(cov);
        }
      }
    }
  }

  return result;
}
