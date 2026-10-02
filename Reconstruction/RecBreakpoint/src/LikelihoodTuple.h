#ifndef RECBREAKPOINT_LIKELIHOODTUPLE_H
#define RECBREAKPOINT_LIKELIHOODTUPLE_H

#include "TrackLikelihood.h"
#include <string>

class TTree;
namespace breakpoint {
struct FitResult;

/// One normalized full-track likelihood, with an explicit validity flag.
/// The quadratic is the complete RTS measurement + process + seed score.
class LikelihoodTuple {
public:
  void book(TTree& tree, const std::string& prefix);
  void reset();
  void assign(const FitResult& result);
  void assign(const TrackLikelihoodResult& result);
  void invalidate(const std::string& error);
private:
  int m_status = 0; // 1 valid, 0 no track, -1 unavailable or evaluation failed
  double m_nll2 = 0, m_quadratic = 0, m_logdet = 0;
  int m_measurementDimensions = 0, m_latentDimensions = 0;
  std::string m_error;
};
} // namespace breakpoint
#endif
