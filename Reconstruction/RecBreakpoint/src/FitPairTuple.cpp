#include "FitPairTuple.h"
#include "TTree.h"
#include <cmath>
#include <limits>
#include <numeric>

namespace breakpoint {
void FitPairTuple::Endpoint::book(TTree& tree, const std::string& prefix) {
  tree.Branch((prefix + "pt").c_str(), &pt);
  tree.Branch((prefix + "ip_parameters").c_str(), &parameters);
  tree.Branch((prefix + "ip_covariance").c_str(), &covariance);
  tree.Branch((prefix + "fitted_log_loss").c_str(), &loss);
  tree.Branch((prefix + "fitted_log_loss_variance").c_str(), &lossVariance);
}

void FitPairTuple::Endpoint::assign(const FitResult& fit, double bz) {
  const auto& ip = fit.ip;
  pt = std::abs(bz * 2.99792458e-4 / ip.omega);
  parameters = {ip.D0, ip.phi, ip.omega, ip.Z0, ip.tanLambda};
  covariance.assign(ip.covMatrix.begin(), ip.covMatrix.end());
  for (const auto& item : fit.breakpoints) {
    loss.push_back(item.fittedLogLoss);
    lossVariance.push_back(item.fittedVariance);
  }
}

void FitPairTuple::States::book(TTree& tree, const std::string& prefix) {
  tree.Branch((prefix + "parameters").c_str(), &parameters);
  tree.Branch((prefix + "covariance").c_str(), &covariance);
}

void FitPairTuple::States::assign(const std::vector<TrackState>& states) {
  for (const auto& state : states) {
    for (int row = 0; row < 5; ++row) {
      parameters.push_back(state.mean(row, 0));
      for (int column = 0; column < 5; ++column)
        covariance.push_back(state.covariance(row, column));
    }
  }
}

void FitPairTuple::book(TTree& tree, const std::string& prefix) {
  tree.Branch((prefix + "result_status").c_str(), &m_status);
  tree.Branch((prefix + "rts_index").c_str(), &m_rtsIndex);
  tree.Branch((prefix + "backward_index").c_str(), &m_backwardIndex);
  m_rts.book(tree, prefix + "rts_");
  m_backward.book(tree, prefix + "backward_");
  tree.Branch((prefix + "breakpoint_interval").c_str(), &m_intervals);
  tree.Branch((prefix + "forward_chi2").c_str(), &m_forwardChi2);
  tree.Branch((prefix + "backward_chi2").c_str(), &m_backwardChi2);
  tree.Branch((prefix + "smoothed_chi2").c_str(), &m_smoothedChi2);
  tree.Branch((prefix + "smoothed_chi2_status").c_str(), &m_smoothedStatus);
  tree.Branch((prefix + "smoothed_chi2_error").c_str(), &m_smoothedError);
  tree.Branch((prefix + "forward_local_chi2").c_str(), &m_forwardLocal);
  tree.Branch((prefix + "backward_local_chi2").c_str(), &m_backwardLocal);
  tree.Branch((prefix + "smoothed_local_chi2").c_str(), &m_smoothedLocal);
  m_forwardPredicted.book(tree, prefix + "forward_predicted_");
  m_forwardFiltered.book(tree, prefix + "forward_filtered_");
  m_smoothed.book(tree, prefix + "smoothed_");
  m_backwardPredicted.book(tree, prefix + "backward_predicted_");
  m_backwardFiltered.book(tree, prefix + "backward_filtered_");
}

void FitPairTuple::reset() {
  // Assignment preserves member addresses registered with ROOT.
  *this = FitPairTuple{};
  const double nan = std::numeric_limits<double>::quiet_NaN();
  m_rts.pt = m_backward.pt = m_forwardChi2 = m_backwardChi2 = m_smoothedChi2 = nan;
}

void FitPairTuple::assign(const PairedFitResult& pair, double bz, int status,
                          int rtsIndex, int backwardIndex) {
  reset();
  m_rts.assign(pair.rts, bz);
  m_backward.assign(pair.backward, bz);
  m_status = status; m_rtsIndex = rtsIndex; m_backwardIndex = backwardIndex;
  m_forwardChi2 = pair.rts.chi2;
  m_backwardChi2 = std::accumulate(pair.backward.backwardChi2.begin(), pair.backward.backwardChi2.end(), 0.);
  m_smoothedChi2 = pair.rts.smoothedTotalChi2;
  m_smoothedStatus = pair.rts.smoothedChi2Status;
  m_smoothedError = pair.rts.smoothedChi2Error;
  for (const auto& loss : pair.rts.breakpoints) m_intervals.push_back(loss.index);
  m_forwardLocal = pair.rts.localChi2;
  m_backwardLocal = pair.backward.backwardChi2;
  m_smoothedLocal = pair.rts.smoothedChi2;
  m_forwardPredicted.assign(pair.rts.predicted);
  m_forwardFiltered.assign(pair.rts.filtered);
  m_smoothed.assign(pair.rts.smoothed);
  m_backwardPredicted.assign(pair.backward.backwardPredicted);
  m_backwardFiltered.assign(pair.backward.backwardFiltered);
}
} // namespace breakpoint
