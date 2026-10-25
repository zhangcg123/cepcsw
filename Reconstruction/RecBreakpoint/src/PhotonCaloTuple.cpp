#include "PhotonCaloProjection.h"
#include "BreakpointFitter.h"
#include "TTree.h"
#include <cmath>
#include <limits>

namespace breakpoint {
void PhotonCaloTuple::book(TTree& tree, const std::string& prefix) {
  auto branch=[&](const char* name, auto* value) { tree.Branch((prefix+name).c_str(),value); };
  branch("status", &m_status); branch("error", &m_error);
  branch("breakpoint_hit", &m_hit); branch("surface_index", &m_surface);
  branch("fitted_log_loss", &m_b); branch("energy_GeV", &m_energy);
  branch("origin_mm", &m_origin); branch("direction", &m_direction);
  branch("position_mm", &m_position); branch("covariance_mm2", &m_covariance);
  branch("z_phi_covariance", &m_zPhiCovariance);
  branch("source_mean", &m_mean); branch("source_covariance", &m_sourceCovariance);
  branch("source_pivot_mm", &m_pivot); branch("position_derivative", &m_derivative);
}

void PhotonCaloTuple::reset() {
  m_status=0; m_hit=m_surface=-1; m_error.clear();
  m_b=m_energy=std::numeric_limits<double>::quiet_NaN();
  for (auto* v : {&m_origin,&m_direction,&m_position,&m_covariance,&m_zPhiCovariance,
                  &m_mean,&m_sourceCovariance,&m_pivot,&m_derivative}) v->clear();
}

void PhotonCaloTuple::assign(const FitResult& fit, const KalmanAdapter& adapter) {
  reset();
  if (fit.breakpoints.empty()) return;
  try {
    if (fit.breakpoints.size()!=1) throw std::runtime_error("Photon projection requires one breakpoint");
    m_hit=fit.breakpoints.front().index;
    if (m_hit<0 || m_hit>=static_cast<int>(fit.smoothed.size()))
      throw std::runtime_error("Photon source has no smoothed upstream state");
    const auto& source=fit.smoothed[m_hit];
    const auto projection=projectPhotonToCalorimeter(source,adapter.calorimeterSurfaces());
    m_b=fit.breakpoints.front().fittedLogLoss;
    const double before=std::hypot(1.,source.mean(4,0))/std::abs(source.mean(2,0));
    m_energy=before*(-std::expm1(-m_b));
    m_surface=projection.surface;
    auto xyz=[](const TVector3& v) { return std::vector<double>{v.X(),v.Y(),v.Z()}; };
    m_origin=xyz(projection.origin); m_direction=xyz(projection.direction);
    m_position=xyz(projection.position);
    m_pivot={source.pivot.x,source.pivot.y,source.pivot.z};
    auto flatten=[](const TMatrixD& matrix, std::vector<double>& output) {
      for (int i=0;i<matrix.GetNrows();++i)
        for (int j=0;j<matrix.GetNcols();++j) output.push_back(matrix(i,j));
    };
    flatten(source.mean,m_mean); flatten(source.covariance,m_sourceCovariance);
    flatten(projection.derivative,m_derivative); flatten(projection.covariance,m_covariance);
    flatten(projection.zPhiCovariance,m_zPhiCovariance);
    m_status=1;
  } catch (const std::exception& error) {
    reset(); m_status=-1; m_error=error.what();
  }
}
} // namespace breakpoint
