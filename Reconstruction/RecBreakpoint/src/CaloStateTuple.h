#ifndef RECBREAKPOINT_CALOSTATETUPLE_H
#define RECBREAKPOINT_CALOSTATETUPLE_H

#include "edm4hep/Track.h"
#include "edm4hep/TrackState.h"

#include <string>
#include <vector>

class TTree;

namespace breakpoint {

/// One calorimeter-face state per published endpoint, in the input-track row.
/// Status: 0 no output, 1 propagated breakpoint state, 2 copied input-KF
/// state, -1 calorimeter propagation failed (the IP fit is still published).
class CaloStateTuple {
public:
  void book(TTree& tree, const std::string& prefix);
  void reset();
  void assign(const edm4hep::TrackState& state, double bz, int status);
  void assignFromTrack(const edm4hep::Track& track, double bz, int status);
  void fail(const std::string& error);
  int status() const { return m_status; }
  const std::string& error() const { return m_error; }

private:
  int m_status = 0;
  std::string m_error;
  double m_pt = 0, m_ptError = 0, m_p = 0;
  // EDM helix order: D0, phi, omega, Z0, tanLambda; covariance is the
  // original packed 21-element EDM matrix. Reference point is in mm.
  std::vector<double> m_parameters, m_covariance, m_referencePoint;
};

} // namespace breakpoint
#endif
