#ifndef RecGsfTracking_GsfComponent_h
#define RecGsfTracking_GsfComponent_h

#include "kaltest/TKalTrack.h"
#include "kaltest/THelicalTrack.h"
#include "TMatrixD.h"
#include "TVector3.h"

#include <string>

/// One Gaussian component in the GSF mixture.
/// Owns a TKalTrack holding the KF state across all measurement sites.
struct GsfComponent {
  double    weight = 1.0;
  int       charge = 1;
  TKalTrack* kaltrack = nullptr;
  double    fitChi2 = 0.0;
  int       debugId = 0;
  int       debugParentId = -1;
  int       generation = 0;
  int       hitsSinceSplit = 0;
  std::string debugHistory;

  ~GsfComponent();

  /// Extract helix from this component's current (last) site
  THelicalTrack helixAtLastSite(double bzTesla) const;

  /// 5×5 covariance at the last site
  TMatrixD covAtLastSite() const;

  /// Deep-clone: copies the full TKalTrack (all sites + states)
  GsfComponent* clone() const;
};

#endif
