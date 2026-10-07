#ifndef RECBREAKPOINT_FITPAIRTUPLE_H
#define RECBREAKPOINT_FITPAIRTUPLE_H

#include "BreakpointFitter.h"
#include "edm4hep/Track.h"
#include <string>
class TTree;

namespace breakpoint {
/// A named pair of endpoint results. Serialization only; never runs a fit.
/// Status: 0 absent, 1 ordinary copy, 2 optimized pair, 3 input KF fallback.
class FitPairTuple {
public:
  void book(TTree& tree, const std::string& prefix);
  void reset();
  void assign(const PairedFitResult& pair, double bz, int status, int rtsIndex, int backwardIndex);
  void assignKF(const edm4hep::Track& track, double bz, int rtsIndex, int backwardIndex);
  int status() const { return m_status; }
  int rtsIndex() const { return m_rtsIndex; }
  int backwardIndex() const { return m_backwardIndex; }
private:
  struct Endpoint {
    double pt = 0, ptError = 0;
    std::vector<double> parameters, covariance, loss, lossVariance;
    void book(TTree& tree, const std::string& prefix);
    void assign(const FitResult& fit, double bz);
  };
  struct States {
    std::vector<double> parameters, covariance;
    void book(TTree& tree, const std::string& prefix);
    void assign(const std::vector<TrackState>& states);
  };
  int m_status = 0, m_rtsIndex = -1, m_backwardIndex = -1;
  Endpoint m_rts, m_backward;
  double m_forwardChi2 = 0, m_backwardChi2 = 0, m_smoothedChi2 = 0;
  double m_kfChi2 = 0; // Only for status 3; never mislabeled as an RTS score.
  int m_smoothedStatus = 0;
  std::string m_smoothedError;
  std::vector<int> m_intervals;
  std::vector<double> m_forwardLocal, m_backwardLocal, m_smoothedLocal;
  // Complete RTS score terms; native measurement is a separate cross-check.
  std::vector<double> m_smoothedMeasurement, m_smoothedProcess, m_smoothedNative;
  double m_smoothedSeed = 0;
  States m_forwardPredicted, m_forwardFiltered, m_smoothed;
  States m_backwardPredicted, m_backwardFiltered;
};
} // namespace breakpoint
#endif
