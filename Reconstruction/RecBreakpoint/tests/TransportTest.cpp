#include "RecBreakpoint/AugmentedTransport.h"
#include "RecBreakpoint/SeedHitSelection.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace breakpoint;

void near(double actual, double expected) {
  if (std::abs(actual - expected) > 1.0e-12)
    throw std::runtime_error("transport regression failed");
}

int main() {
  const auto wide = parseSeedHitSelection("FirstMiddleLast");
  const auto legacy = parseSeedHitSelection("FirstThree");
  // Original ordered-hit indices need not be consecutive after excluding 1D hits.
  if (selectSeedHitIndices({0, 2, 4, 6, 8}, wide) != std::array<int,3>{0,4,8} ||
      selectSeedHitIndices({1, 3, 7, 9}, wide) != std::array<int,3>{1,7,9} ||
      selectSeedHitIndices({0, 4, 8}, wide) != std::array<int,3>{0,4,8} ||
      selectSeedHitIndices({1, 3, 7, 9}, legacy) != std::array<int,3>{1,3,7})
    throw std::runtime_error("seed-hit selection regression failed");
  bool invalidModeRejected = false, insufficientHitsRejected = false;
  try { parseSeedHitSelection("Middle"); }
  catch (const std::invalid_argument&) { invalidModeRejected = true; }
  try { selectSeedHitIndices({0, 2}, wide); }
  catch (const std::runtime_error&) { insufficientHitsRejected = true; }
  if (!invalidModeRejected || !insufficientHitsRejected)
    throw std::runtime_error("invalid seed selection accepted");
  Matrix5 identity{};
  Matrix6 prior{};
  for (int i = 0; i < 5; ++i) identity[i * 5 + i] = 1;
  for (int i = 0; i < 6; ++i) prior[i * 6 + i] = i + 1;
  prior[2 * 6 + 5] = prior[5 * 6 + 2] = 0.25;
  const auto ordinary = AugmentedTransport::jacobian(identity, {});
  const auto unchanged = AugmentedTransport::covariance(prior, ordinary, {});
  for (int i = 0; i < 36; ++i) near(unchanged[i], prior[i]);

  Vector5 derivative{};
  derivative[2] = 2;
  const auto coupled = AugmentedTransport::jacobian(identity, derivative);
  const auto updated = AugmentedTransport::covariance(prior, coupled, {});
  near(updated[2 * 6 + 2], 3 + 4 * 6 + 4 * 0.25);
  near(updated[2 * 6 + 5], 0.25 + 2 * 6);
  near(updated[35], 6);

  // Inverse shear restores the complete prior, including cross correlations.
  derivative[2] = -2;
  const auto restored = AugmentedTransport::covariance(
      updated, AugmentedTransport::jacobian(identity, derivative), {});
  for (int i = 0; i < 36; ++i) near(restored[i], prior[i]);

  Matrix5 q5{};
  q5[0] = 0.125;
  const auto noisy = AugmentedTransport::covariance(
      prior, ordinary, AugmentedTransport::processNoise(q5));
  near(noisy[0], 1.125);
  near(noisy[35], 6);

  // Persistent transport: b is not reapplied at subsequent surfaces. Its
  // variance stays constant, but track/b correlations follow the geometry.
  Matrix5 downstream = identity;
  downstream[2] = 0.4; // local position depends on curvature
  const auto downstreamJ = AugmentedTransport::jacobian(downstream, {});
  const auto downstreamQ = AugmentedTransport::processNoise(q5);
  for (int i = 0; i < 5; ++i) {
    near(downstreamJ[i * 6 + 5], 0);
    near(downstreamJ[5 * 6 + i], 0);
    near(downstreamQ[i * 6 + 5], 0);
    near(downstreamQ[5 * 6 + i], 0);
  }
  near(downstreamJ[35], 1);
  near(downstreamQ[35], 0);
  auto carried = updated;
  for (int hit = 0; hit < 20; ++hit)
    carried = AugmentedTransport::covariance(carried, downstreamJ, downstreamQ);
  near(carried[35], updated[35]);
  near(carried[5], updated[5] + 20 * 0.4 * updated[17]);

  // Physical loss-map derivatives: kappa' = exp(b) kappa.
  const double kappa = -0.2, b = 0.07, epsilon = 1.e-6;
  const double derivativeB = kappa * std::exp(b);
  if (std::abs((kappa * std::exp(b + epsilon) - kappa * std::exp(b - epsilon)) /
               (2 * epsilon) - derivativeB) > 1.e-10)
    throw std::runtime_error("loss derivative finite-difference test failed");
  Matrix5 lossMap = identity;
  lossMap[12] = std::exp(b);
  Vector5 lossColumn{};
  lossColumn[2] = derivativeB;
  const auto lossJacobian = AugmentedTransport::jacobian(lossMap, lossColumn);
  Matrix5 inverseMap = identity;
  inverseMap[12] = std::exp(-b);
  Vector5 inverseColumn{};
  inverseColumn[2] = -kappa;
  const auto inverseJacobian = AugmentedTransport::jacobian(inverseMap, inverseColumn);
  const auto physicalRestored = AugmentedTransport::covariance(
      AugmentedTransport::covariance(prior, lossJacobian, {}), inverseJacobian, {});
  for (int i = 0; i < 36; ++i) near(physicalRestored[i], prior[i]);

  // An iterated affine loss map must retain the ORIGINAL prior mean. At a
  // changed expansion point the offset is essential; merely applying exp(bref)
  // to the old kappa would silently replace the loss hypothesis.
  const double priorB = 0, referenceB = .12, referenceKappa = -.19;
  const double referenceMapped = referenceKappa * std::exp(referenceB);
  const double affinePrediction = referenceMapped
      + std::exp(referenceB) * (kappa-referenceKappa)
      + referenceMapped * (priorB-referenceB);
  near(affinePrediction, std::exp(referenceB)
      * (kappa + referenceKappa*(priorB-referenceB)));
  const double inverseReference = referenceKappa * std::exp(-referenceB);
  const double inverseAffine = inverseReference + std::exp(-referenceB)
      * (kappa-referenceKappa) - inverseReference*(priorB-referenceB);
  near(inverseAffine, std::exp(-referenceB)
      * (kappa-referenceKappa*(priorB-referenceB)));
  // For an already-linear measurement, moving only its expansion point
  // cannot change the posterior or repeatedly shrink the original prior.
  const double originalMean=.3, originalVariance=.04, h=2., observation=.8, noise=.01;
  const double posteriorVariance=1./(1./originalVariance+h*h/noise);
  const double posteriorMean=posteriorVariance*(originalMean/originalVariance+h*observation/noise);
  for (double reference : {-.7, .0, .6}) {
    const double prediction=h*reference+h*(originalMean-reference);
    const double gain=originalVariance*h/(h*h*originalVariance+noise);
    near(originalMean+gain*(observation-prediction),posteriorMean);
    near((1-gain*h)*originalVariance,posteriorVariance);
  }

  // Scalar downstream measurement of curvature: conditioning the full joint
  // or retaining b/helix cross covariance gives the same b posterior.
  const auto joint = AugmentedTransport::covariance(prior, coupled, {});
  const double measurementVariance = 0.7, residual = 0.3;
  const double predictedVariance = joint[14];
  const double cross = joint[32]; // Cov(b,kappa)
  const double innovationVariance = predictedVariance + measurementVariance;
  const double directMean = cross / innovationVariance * residual;
  const double directVariance = joint[35] - cross * cross / innovationVariance;
  const double updatedCurvature = predictedVariance / innovationVariance * residual;
  const double updatedVariance = predictedVariance * measurementVariance / innovationVariance;
  const double conditionalGain = cross / predictedVariance;
  near(conditionalGain * updatedCurvature, directMean);
  near(joint[35] + conditionalGain * conditionalGain *
       (updatedVariance - predictedVariance), directVariance);

  // Backward loss/state cross covariance must survive subsequent hit updates.
  // Check two sequential scalar measurements against their joint likelihood.
  const double nextNoise = 1.3, nextObservation = -0.1;
  const double remainingCross = conditionalGain * updatedVariance;
  const double nextInnovation = updatedVariance + nextNoise;
  const double secondMean = directMean + remainingCross / nextInnovation *
      (nextObservation - updatedCurvature);
  const double secondVariance = directVariance - remainingCross * remainingCross / nextInnovation;
  const double combinedNoise = 1. / (1. / measurementVariance + 1. / nextNoise);
  const double combinedObservation = combinedNoise *
      (residual / measurementVariance + nextObservation / nextNoise);
  near(secondMean, cross / (predictedVariance + combinedNoise) * combinedObservation);
  near(secondVariance, joint[35] - cross * cross / (predictedVariance + combinedNoise));
  const double inverseDerivative = -kappa * std::exp(-b);
  if (std::abs((kappa * std::exp(-(b + epsilon)) - kappa * std::exp(-(b - epsilon))) /
               (2 * epsilon) - inverseDerivative) > 1.e-10)
    throw std::runtime_error("inverse loss derivative test failed");

  bool asymmetricRejected = false;
  Matrix6 invalid = prior;
  invalid[1] = 0.5;
  try { AugmentedTransport::covariance(invalid, ordinary, {}); }
  catch (const std::invalid_argument&) { asymmetricRejected = true; }
  if (!asymmetricRejected) throw std::runtime_error("asymmetric input not rejected");
  bool rejected = false;
  prior[0] = std::numeric_limits<double>::quiet_NaN();
  try { AugmentedTransport::covariance(prior, ordinary, {}); }
  catch (const std::invalid_argument&) { rejected = true; }
  if (!rejected) throw std::runtime_error("nonfinite input not rejected");
  std::cout << "RecBreakpoint transport tests passed\n";
}
