#ifndef RECBREAKPOINT_FREELOSSFITTER_H
#define RECBREAKPOINT_FREELOSSFITTER_H

#include "BreakpointFitter.h"
#include "TrackLikelihood.h"
#include <limits>
#include <optional>

namespace breakpoint {

struct FreeLossSettings {
  double maxLogLoss = 1; // b in [0,maxLogLoss]; default maximum loss is 63.2121%
  unsigned maxCallsPerStart = 180;
  double tolerance = .001;
  bool checkEquivalentLikelihoods = false;
};

enum class FreeLossStatus { NotAttempted = 0, NoInterval = 1, Applied = 2,
                            Unsupported = -1, Failed = -2 };
enum class FreeLossTrialPhase { Scan = 0, Minimize = 1, Repeat = 2, LocalScan = 3 };

struct FreeLossTrial {
  double b = 0;
  TrackLikelihoodResult likelihood;
  bool valid = false;
  FreeLossTrialPhase phase = FreeLossTrialPhase::Scan;
  std::string error;
};

struct FreeLossDiagnostics {
  FreeLossStatus status = FreeLossStatus::NotAttempted;
  int interval = -1;
  int minuitStatus = -99; // sentinel: scan winner, not a converged interior minimum
  double b = std::numeric_limits<double>::quiet_NaN();
  double bError = std::numeric_limits<double>::quiet_NaN(); // local Minuit estimate only
  double edm = std::numeric_limits<double>::quiet_NaN();
  TrackLikelihoodResult likelihood;
  bool lowerBound = false, upperBound = false;
  std::string error;
  std::vector<FreeLossTrial> trials;
};

struct FreeLossFitResult {
  std::optional<PairedFitResult> fitted;
  FreeLossDiagnostics diagnostics;
};

/// Outer scalar optimization only. Every conditional trajectory is produced
/// by BreakpointFitter; no Kalman filter/smoother is reimplemented here.
/// No truth-loss input is accepted by this interface.
class FreeLossFitter {
public:
  explicit FreeLossFitter(const BreakpointFitter& fitter) : m_fitter(fitter) {}
  FreeLossFitResult fit(const std::vector<edm4hep::TrackerHit>& hits,
                       const FitSettings& ordinary, const FreeLossSettings& controls) const;
private:
  const BreakpointFitter& m_fitter;
};

} // namespace breakpoint
#endif
