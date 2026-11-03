#ifndef RECBREAKPOINT_HIT_LOCATION_SCAN_H
#define RECBREAKPOINT_HIT_LOCATION_SCAN_H

#include <cmath>
#include <limits>
#include <optional>
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

  // Passive truth-location reference: 0 inactive, 1 null, 2 fitted,
  // -1 truth association failed, -2 fit failed. Never enters select().
  int truthStatus = 0, truthHit = -1, truthMinuitStatus = -99;
  bool truthReusedCandidate = false;
  double truthNll2 = std::numeric_limits<double>::quiet_NaN();
  double truthIpOmega = std::numeric_limits<double>::quiet_NaN();
  double truthPt = std::numeric_limits<double>::quiet_NaN();
  double truthPriorCenter = std::numeric_limits<double>::quiet_NaN();
  double truthFittedLoss = std::numeric_limits<double>::quiet_NaN();
  double truthFittedVariance = std::numeric_limits<double>::quiet_NaN();
  std::string truthError;

  void reset(double minimumGain) { *this = HitLocationScan{}; minNllGain=minimumGain; }
  void book(TTree& tree);
  void run(const BreakpointFitter& fitter, const std::vector<edm4hep::TrackerHit>& hits,
           const FitSettings& settings, const FreeLossSettings& controls);
  void reference(const BreakpointFitter& fitter, const std::vector<edm4hep::TrackerHit>& hits,
                 const FitSettings& settings, const FreeLossSettings& controls, int ownerHit);

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

// Null, reconstructed silicon sites plus the first exact TPC row0, then an
// optional truth-only site. Truth never changes the reconstructed candidates.
inline std::vector<int> locationScanPoints(const std::vector<int>& systems,
    const std::vector<int>& layers, int truthHit=-1) {
  if (systems.size()!=layers.size())
    throw std::invalid_argument("Misaligned location-scan detector and layer IDs");
  std::vector<int> points{-1};
  bool addedTpcRow0=false;
  for (std::size_t i=0;i+1<systems.size();++i) {
    const bool silicon=systems[i]>=1 && systems[i]<=3;
    const bool tpcRow0=systems[i]==4 && layers[i]==0 && !addedTpcRow0;
    if (silicon || tpcRow0) points.push_back(static_cast<int>(i));
    if (tpcRow0) addedTpcRow0=true;
  }
  if (truthHit>=0 && static_cast<std::size_t>(truthHit+1)<systems.size()) {
    bool present=false;
    for (int i : points) if (i==truthHit) present=true;
    if (!present) points.push_back(truthHit);
  }
  return points;
}

/// Diagnostic only: one aligned vector entry per (point, setup). No ranking,
/// NLL-gain cut, selected location or published refitted-track endpoint.
struct LocationFitScan {
  int status=0, pointCount=0, truthStatus=0, truthHit=-1, truthPoint=-1;
  std::string truthError;
  // kind: 0 null, 1 silicon candidate, 2 truth-only reference, 3 TPC row0 candidate.
  std::vector<int> point, kind, hit, system, layer, setup, actualMask;
  std::vector<int> fitStatus, attemptStatus, minuitStatus, attemptMinuitStatus;
  std::vector<int> sourceSetup, measurementDimensions;
  std::vector<double> nll2, chi2, logDeterminant, pt, ptError;
  std::vector<double> priorCenter, fittedLoss, fittedVariance;
  std::vector<std::string> error;

  void reset() { *this=LocationFitScan{}; }
  void book(TTree& tree);
  void run(const BreakpointFitter& fitter, const std::vector<edm4hep::TrackerHit>& hits,
           const FitSettings& observations, const FreeLossSettings& controls,
           double bz, std::optional<int> truthOwner);
};
} // namespace breakpoint
#endif
