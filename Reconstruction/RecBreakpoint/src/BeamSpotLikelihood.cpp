#include "BeamSpotLikelihood.h"
#include "TrackState.h"
#include "kaltest/THelicalTrack.h"
#include "TVector3.h"

#include <cmath>
#include <stdexcept>

namespace breakpoint {

BeamSpotLikelihood evaluateBeamSpotLikelihood(const edm4hep::TrackState& ip,
                                               double bz, const BeamSpotSettings& beam) {
  if (!std::isfinite(beam.x) || !std::isfinite(beam.y) ||
      !std::isfinite(beam.sigmaX) || !std::isfinite(beam.sigmaY) ||
      beam.sigmaX <= 0 || beam.sigmaY <= 0)
    throw std::invalid_argument("Beam-spot position must be finite and widths finite/positive");

  const auto input = fromEDM(ip, bz);
  THelicalTrack helix(input.mean(0, 0), input.mean(1, 0), input.mean(2, 0),
                      input.mean(3, 0), input.mean(4, 0),
                      input.pivot.x, input.pivot.y, input.pivot.z, bz);
  TMatrixD covariance(input.covariance), jacobian(5, 5);
  jacobian.UnitMatrix();
  double angle = 0;
  helix.MoveTo(TVector3(beam.x, beam.y, 0), angle, &jacobian, &covariance);

  // KalTest phi0 points along the local drho normal. The 2D beam ellipse
  // supplies ONE transverse measurement, not an artificial 2D tracker hit.
  const double cosPhi = std::cos(helix.GetPhi0());
  const double sinPhi = std::sin(helix.GetPhi0());
  BeamSpotLikelihood score;
  score.residual = helix.GetDrho();
  score.trackVariance = covariance(0, 0);
  score.beamVariance = cosPhi * cosPhi * beam.sigmaX * beam.sigmaX +
                       sinPhi * sinPhi * beam.sigmaY * beam.sigmaY;
  score.innovationVariance = score.trackVariance + score.beamVariance;
  if (!std::isfinite(score.residual) || !std::isfinite(score.innovationVariance) ||
      score.innovationVariance <= 0 || !std::isfinite(score.trackVariance) ||
      score.trackVariance < 0)
    throw std::runtime_error("Invalid beam-spot predictive likelihood");
  score.quadratic = score.residual * score.residual / score.innovationVariance;
  score.logDeterminant = std::log(score.innovationVariance);
  score.nll2 = score.quadratic + score.logDeterminant + std::log(2 * std::acos(-1.));
  if (!std::isfinite(score.nll2))
    throw std::runtime_error("Nonfinite beam-spot predictive likelihood");
  return score;
}

} // namespace breakpoint
