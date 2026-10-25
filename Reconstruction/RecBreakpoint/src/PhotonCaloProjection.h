#ifndef RECBREAKPOINT_PHOTONCALOPROJECTION_H
#define RECBREAKPOINT_PHOTONCALOPROJECTION_H

#include "TrackState.h"
#include "TVector3.h"
#include <string>
#include <vector>

class TVSurface;
class TTree;

namespace breakpoint {
struct FitResult;
class KalmanAdapter;

struct PhotonCaloProjection {
  TVector3 origin, direction, position;
  TMatrixD derivative{3, 5}; // global ECAL xyz versus the upstream KalTest state
  TMatrixD covariance{3, 3};
  TMatrixD zPhiCovariance{2, 2}; // z [mm], phi [rad], full row-major matrix
  int surface = -1;
};

// Collinear photon from the fitted breakpoint, with no material interactions.
// Its geometry is independent of b: the full smoothed 5D marginal is sufficient.
PhotonCaloProjection projectPhotonToCalorimeter(const TrackState& source,
    const std::vector<const TVSurface*>& surfaces);

class PhotonCaloTuple {
public:
  void book(TTree& tree, const std::string& prefix);
  void reset();
  void assign(const FitResult& fit, const KalmanAdapter& adapter);
private:
  int m_status = 0, m_hit = -1, m_surface = -1;
  std::string m_error;
  double m_b = 0, m_energy = 0;
  std::vector<double> m_origin, m_direction, m_position, m_covariance, m_zPhiCovariance;
  std::vector<double> m_mean, m_sourceCovariance, m_pivot, m_derivative;
};
} // namespace breakpoint
#endif
