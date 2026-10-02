#include "LikelihoodTuple.h"
#include "BreakpointFitter.h"
#include "TTree.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace breakpoint {
void LikelihoodTuple::book(TTree& tree, const std::string& prefix) {
  tree.Branch((prefix + "status").c_str(), &m_status);
  tree.Branch((prefix + "nll2").c_str(), &m_nll2);
  tree.Branch((prefix + "quadratic").c_str(), &m_quadratic);
  tree.Branch((prefix + "logdet").c_str(), &m_logdet);
  tree.Branch((prefix + "measurement_dimensions").c_str(), &m_measurementDimensions);
  tree.Branch((prefix + "latent_dimensions").c_str(), &m_latentDimensions);
  tree.Branch((prefix + "error").c_str(), &m_error);
}

void LikelihoodTuple::reset() {
  m_status = 0;
  m_nll2 = m_quadratic = m_logdet = std::numeric_limits<double>::quiet_NaN();
  m_measurementDimensions = m_latentDimensions = 0;
  m_error.clear();
}

void LikelihoodTuple::invalidate(const std::string& error) {
  reset();
  m_status = -1;
  m_error = error;
}

void LikelihoodTuple::assign(const TrackLikelihoodResult& result) {
  if (!std::isfinite(result.nll2) || !std::isfinite(result.quadratic) ||
      !std::isfinite(result.logDeterminant) || result.measurementDimensions <= 0)
    throw std::runtime_error("Invalid normalized track likelihood");
  m_status = 1;
  m_nll2 = result.nll2;
  m_quadratic = result.quadratic;
  m_logdet = result.logDeterminant;
  m_measurementDimensions = result.measurementDimensions;
  m_latentDimensions = result.latentDimensions;
  m_error.clear();
}

void LikelihoodTuple::assign(const FitResult& result) {
  try {
    if (result.smoothedChi2Status != 1)
      throw std::runtime_error("Complete RTS score unavailable: " + result.smoothedChi2Error);
    if (!result.gaussianModel)
      throw std::runtime_error("Gaussian model unavailable: " + result.gaussianModelError);
    assign(evaluateSmoothedTrackLikelihood(*result.gaussianModel,
                                           result.smoothedTotalChi2));
  } catch (const std::exception& error) {
    invalidate(error.what()); // a diagnostic must not discard the track
  }
}
} // namespace breakpoint
