#include "CaloStateTuple.h"
#include "TrackState.h"

#include "TTree.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace breakpoint {

void CaloStateTuple::book(TTree& tree, const std::string& prefix) {
  tree.Branch((prefix + "calo_status").c_str(), &m_status);
  tree.Branch((prefix + "calo_error").c_str(), &m_error);
  tree.Branch((prefix + "calo_pt").c_str(), &m_pt);
  tree.Branch((prefix + "calo_pt_error").c_str(), &m_ptError);
  tree.Branch((prefix + "calo_p").c_str(), &m_p);
  tree.Branch((prefix + "calo_parameters").c_str(), &m_parameters);
  tree.Branch((prefix + "calo_covariance").c_str(), &m_covariance);
  tree.Branch((prefix + "calo_reference_point_mm").c_str(), &m_referencePoint);
}

void CaloStateTuple::reset() {
  m_status = 0;
  m_error.clear();
  m_pt = m_ptError = m_p = std::numeric_limits<double>::quiet_NaN();
  m_parameters.clear();
  m_covariance.clear();
  m_referencePoint.clear();
}

void CaloStateTuple::assign(const edm4hep::TrackState& state, double bz, int status) {
  if (state.location != edm4hep::TrackState::AtCalorimeter ||
      !std::isfinite(state.omega) || state.omega == 0 ||
      !std::isfinite(state.tanLambda) || !std::isfinite(bz))
    throw std::runtime_error("Invalid AtCalorimeter track state");
  reset();
  m_status = status;
  m_parameters = {state.D0, state.phi, state.omega, state.Z0, state.tanLambda};
  m_covariance.assign(state.covMatrix.begin(), state.covMatrix.end());
  m_referencePoint = {state.referencePoint.x, state.referencePoint.y, state.referencePoint.z};
  m_pt = std::abs(bz * 2.99792458e-4 / state.omega);
  m_ptError = transverseMomentumError(state, bz);
  m_p = m_pt * std::hypot(1.0, static_cast<double>(state.tanLambda));
}

void CaloStateTuple::assignFromTrack(const edm4hep::Track& track, double bz, int status) {
  for (const auto& state : track.getTrackStates())
    if (state.location == edm4hep::TrackState::AtCalorimeter) {
      try { assign(state, bz, status); }
      catch (const std::exception& error) { fail(error.what()); }
      return;
    }
  fail("Input track has no AtCalorimeter state");
}

void CaloStateTuple::fail(const std::string& error) {
  reset();
  m_status = -1;
  m_error = error;
}

} // namespace breakpoint
