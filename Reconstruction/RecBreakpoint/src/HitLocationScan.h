#ifndef RECBREAKPOINT_HIT_LOCATION_SCAN_H
#define RECBREAKPOINT_HIT_LOCATION_SCAN_H

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

class TTree;
namespace edm4hep { class TrackerHit; }
namespace breakpoint {
class BreakpointFitter;
struct FitSettings;
struct FreeLossSettings;

// TDR/ILD subdetector IDs: VXD=1, ITK barrel=2, ITK endcap=3.
inline std::vector<int> siliconScanHits(const std::vector<int>& systems) {
  std::vector<int> hits;
  for (std::size_t i=0; i+1<systems.size(); ++i)
    if (systems[i]>=1 && systems[i]<=3) hits.push_back(static_cast<int>(i));
  return hits;
}

/// Location selection with configured beam/charged observations. No truth,
/// separate neutral measurement or earlier fit posterior.
/// Candidate vectors remain aligned, including failed optimizations.
struct HitLocationScan {
  // 0 not run, 1 null preferred, 2 breakpoint preferred, 3 no silicon candidates,
  // -1 null failed, -2 all candidate optimizations failed. Partial failures are
  // counted separately: a minimum over successful candidates is not exhaustive.
  int status = 0, bestHit = -1, selectedHit = -1, failedCandidates = 0, measurementDimensions = 0;
  int constraintMask = 0; // beam=1, charged=2; separate neutral is never scored
  double minNllGain = 0;
  double nullNll2 = std::numeric_limits<double>::quiet_NaN();
  double nullIpOmega = std::numeric_limits<double>::quiet_NaN();
  double bestNll2 = std::numeric_limits<double>::quiet_NaN();
  double secondNll2 = std::numeric_limits<double>::quiet_NaN();
  double gain = std::numeric_limits<double>::quiet_NaN();
  std::string error;
  std::string chargedEnergyMode = "None"; // candidate mode; null is always electron-only
  std::vector<int> hit, system, layer, fitStatus, minuitStatus;
  std::vector<double> nll2, priorCenter, fittedLoss, fittedVariance, ipOmega;
  std::vector<std::string> candidateError;

  void reset(double minimumGain) { *this = HitLocationScan{}; minNllGain=minimumGain; }
  void book(TTree& tree);
  void run(const BreakpointFitter& fitter, const std::vector<edm4hep::TrackerHit>& hits,
           const FitSettings& settings, const FreeLossSettings& controls);

  // Pure decision step, also exercised without Gaudi/geometry in unit tests.
  void select() {
    if (!std::isfinite(minNllGain) || minNllGain<0)
      throw std::invalid_argument("HitScanMinNllGain must be finite and nonnegative");
    status=0; bestHit=selectedHit=-1; failedCandidates=0;
    bestNll2=secondNll2=gain=std::numeric_limits<double>::quiet_NaN();
    if (hit.size()!=nll2.size() || hit.size()!=fitStatus.size())
      throw std::invalid_argument("Misaligned hit-scan scores");
    if (!std::isfinite(nullNll2)) { status=-1; return; }
    if (hit.empty()) { status=3; return; }
    for (std::size_t i=0; i<hit.size(); ++i) {
      if (fitStatus[i]!=2 || !std::isfinite(nll2[i])) { ++failedCandidates; continue; }
      if (bestHit<0 || nll2[i]<bestNll2) {
        secondNll2=bestNll2; bestNll2=nll2[i]; bestHit=hit[i];
      } else if (!std::isfinite(secondNll2) || nll2[i]<secondNll2) secondNll2=nll2[i];
    }
    if (bestHit<0) { status=-2; return; }
    gain=nullNll2-bestNll2;
    status=gain>minNllGain ? 2 : 1; // Ties and insufficient gains prefer no breakpoint.
    if (status==2) selectedHit=bestHit;
  }
};
} // namespace breakpoint
#endif
