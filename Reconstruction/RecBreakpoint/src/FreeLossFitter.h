#ifndef RECBREAKPOINT_FREELOSSFITTER_H
#define RECBREAKPOINT_FREELOSSFITTER_H

#include "BreakpointFitter.h"
#include "BeamSpotLikelihood.h"
#include "TrackLikelihood.h"
#include <limits>
#include <optional>

namespace breakpoint {

struct FreeLossSettings {
  double maxLogLoss = 1; // prior center in [0,maxLogLoss], not a posterior bound
  unsigned maxCallsPerStart = 180;
  double tolerance = .001;
  std::optional<BeamSpotSettings> beamSpot; // objective only; never a KF hit
};

enum class FreeLossStatus { NotAttempted = 0, NoInterval = 1, Applied = 2,
                            Unsupported = -1, Failed = -2 };
enum class FreeLossTrialPhase { Scan = 0, Minimize = 1, Repeat = 2, LocalScan = 3 };

struct FreeLossTrial {
  double b = 0; // trial prior center; retained name for tuple compatibility
  TrackLikelihoodResult likelihood;
  BeamSpotLikelihood beam;
  double objectiveNll2 = std::numeric_limits<double>::quiet_NaN();
  bool valid = false;
  FreeLossTrialPhase phase = FreeLossTrialPhase::Scan;
  std::string error;
};

struct FreeLossDiagnostics {
  FreeLossStatus status = FreeLossStatus::NotAttempted;
  int interval = -1;
  int minuitStatus = -99; // sentinel: no accepted converged minimum
  double b = std::numeric_limits<double>::quiet_NaN(); // optimized prior center
  double bError = std::numeric_limits<double>::quiet_NaN(); // Minuit center error, NOT sigma_b
  double priorSigmaLogLoss = std::numeric_limits<double>::quiet_NaN();
  double edm = std::numeric_limits<double>::quiet_NaN();
  TrackLikelihoodResult likelihood;
  BeamSpotLikelihood beam;
  double objectiveNll2 = std::numeric_limits<double>::quiet_NaN();
  bool lowerBound = false, upperBound = false;
  std::string error;
  std::vector<FreeLossTrial> trials;
};

struct FreeLossFitResult {
  std::optional<PairedFitResult> fitted;
  FreeLossDiagnostics diagnostics;
};

/// Outer scalar prior-center optimization only. Every trajectory is produced
/// by BreakpointFitter; no Kalman filter/smoother is reimplemented here.
/// Every trial/final fit retains ordinary.sigmaLogLoss; posterior b can move.
/// No truth-loss input is accepted by this interface.
class FreeLossFitter {
public:
  FreeLossFitter(const BreakpointFitter& fitter, double bz) : m_fitter(fitter), m_bz(bz) {}
  FreeLossFitResult fit(const std::vector<edm4hep::TrackerHit>& hits,
                       const FitSettings& ordinary, const FreeLossSettings& controls) const;
private:
  const BreakpointFitter& m_fitter;
  double m_bz;
};

} // namespace breakpoint
#endif
