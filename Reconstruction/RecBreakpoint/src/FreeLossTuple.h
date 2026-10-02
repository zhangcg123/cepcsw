#ifndef RECBREAKPOINT_FREELOSSTUPLE_H
#define RECBREAKPOINT_FREELOSSTUPLE_H

#include "FreeLossFitter.h"
#include <string>
class TTree;

namespace breakpoint {
/// Tuple serialization only, kept out of both the optimizer and Gaudi flow.
class FreeLossTuple {
public:
  void book(TTree& tree, const std::string& prefix = "free_loss_");
  void reset(bool enabled, const FreeLossSettings& controls, double sigmaLogLoss);
  void assign(const FreeLossDiagnostics& diagnostic);
  bool applied() const { return m_status == int(FreeLossStatus::Applied); }
private:
  bool m_enabled = false, m_applied = false, m_lower = false, m_upper = false;
  bool m_covarianceConditional = false;
  std::string m_lossTreatment = "PriorCenter";
  double m_priorSigmaLogLoss = 0;
  double m_maxLogLoss = 1, m_tolerance = .001;
  int m_maxCalls = 180;
  int m_status = 0, m_interval = -1, m_minuitStatus = -99;
  double m_b = 0, m_errorB = 0, m_edm = 0, m_nll2 = 0, m_quadratic = 0, m_logdet = 0;
  double m_objectiveNll2 = 0, m_beamNll2 = 0, m_beamQuadratic = 0, m_beamLogdet = 0;
  double m_beamResidual = 0, m_beamVariance = 0, m_beamTrackVariance = 0, m_beamSpotVariance = 0;
  std::string m_error;
  std::vector<double> m_trialB, m_trialNll2, m_trialObjectiveNll2, m_trialBeamNll2;
  std::vector<int> m_trialValid, m_trialPhase;
  std::vector<std::string> m_trialError;
};
} // namespace breakpoint
#endif
