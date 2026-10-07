#ifndef RECBREAKPOINT_TRACKSTATE_H
#define RECBREAKPOINT_TRACKSTATE_H

#include "TMatrixD.h"
#include "edm4hep/TrackState.h"
#include "TrackSystemSvc/IMarlinTrack.h"

namespace breakpoint {

/// KalTest coordinates: drho [mm], phi0 [rad], kappa [1/GeV], dz [mm], tanl.
/// The sixth coordinate of a native KalTest state is deliberately excluded.
struct TrackState {
  TMatrixD mean{5, 1};
  TMatrixD covariance{5, 5};
  edm4hep::Vector3d pivot{};
};

TrackState fromEDM(const edm4hep::TrackState& state, double bz);
edm4hep::TrackState toEDM(const TrackState& state, double bz, int location = 0);
/// One-sigma pT uncertainty [GeV] from the EDM omega variance; NaN if invalid.
double transverseMomentumError(const edm4hep::TrackState& state, double bz);
TMatrixD transpose(const TMatrixD& matrix);
TMatrixD inverseCovariance(const TMatrixD& matrix);
TMatrixD stateDifference(const TMatrixD& left, const TMatrixD& right);
void validateCovariance(TMatrixD& matrix);

} // namespace breakpoint
#endif
