#include "PhotonCaloProjection.h"
#include "kaltest/TStraightTrack.h"
#include "kaltest/TVSurface.h"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace breakpoint {
PhotonCaloProjection projectPhotonToCalorimeter(const TrackState& source,
    const std::vector<const TVSurface*>& surfaces) {
  if (source.mean.GetNrows() != 5 || source.covariance.GetNrows() != 5 ||
      source.covariance.GetNcols() != 5)
    throw std::invalid_argument("Photon projection requires a full 5D track marginal");
  for (int i=0; i<5; ++i) {
    if (!std::isfinite(source.mean(i, 0)))
      throw std::runtime_error("Nonfinite photon source mean");
    for (int j=0; j<5; ++j)
      if (!std::isfinite(source.covariance(i, j)))
        throw std::runtime_error("Nonfinite photon source covariance");
  }
  const TVector3 pivot(source.pivot.x, source.pivot.y, source.pivot.z);
  const TStraightTrack ray(source.mean, pivot);
  PhotonCaloProjection result;
  result.origin = ray.CalcXAt(0);
  const auto velocity = ray.CalcDxDphi(0);
  result.direction = TVector3(velocity(0,0), velocity(1,0), velocity(2,0)).Unit();
  const TVSurface* destination = nullptr;
  double nearest = std::numeric_limits<double>::infinity();
  for (std::size_t i=0; i<surfaces.size(); ++i) {
    if (!surfaces[i]) continue;
    double distance = 0;
    TVector3 point;
    // Bypass charged-helix specializations; retain native shape and boundary checks.
    if (surfaces[i]->TVSurface::CalcXingPointWith(ray, point, distance, 0, 1.e-7) > 0 &&
        std::isfinite(distance) && distance > 0 && distance < nearest) {
      nearest = distance;
      destination = surfaces[i];
      result.position = point;
      result.surface = i;
    }
  }
  if (!destination) throw std::runtime_error("No forward photon ECAL-face intersection");
  const auto partial = ray.CalcDxDa(nearest);
  const auto tangent = ray.CalcDxDphi(nearest);
  const auto normal = destination->CalcDSDx(result.position);
  const double slope = (normal * tangent)(0,0);
  if (!std::isfinite(slope) || std::abs(slope) < 1.e-12)
    throw std::runtime_error("Photon ray is tangent to ECAL surface");
  result.derivative = partial - tangent * (normal * partial) * (1. / slope);
  result.covariance = result.derivative * source.covariance *
      TMatrixD(TMatrixD::kTransposed, result.derivative);
  const double r2 = result.position.Perp2();
  if (!(r2 > 0)) throw std::runtime_error("Undefined photon ECAL azimuth");
  TMatrixD zPhi(2,3);
  zPhi.Zero();
  zPhi(0,2) = 1;
  zPhi(1,0) = -result.position.Y()/r2;
  zPhi(1,1) = result.position.X()/r2;
  result.zPhiCovariance = zPhi * result.covariance * TMatrixD(TMatrixD::kTransposed,zPhi);
  for (int i=0; i<2; ++i)
    for (int j=0; j<2; ++j)
      if (!std::isfinite(result.zPhiCovariance(i,j)))
        throw std::runtime_error("Invalid photon ECAL covariance");
  const double a=result.zPhiCovariance(0,0), b=result.zPhiCovariance(1,1);
  const double cross=result.zPhiCovariance(0,1);
  if (a < 0 || b < 0 || cross*cross > a*b*(1.+1.e-8)+1.e-18)
    throw std::runtime_error("Nonpositive photon ECAL covariance");
  return result;
}
} // namespace breakpoint
