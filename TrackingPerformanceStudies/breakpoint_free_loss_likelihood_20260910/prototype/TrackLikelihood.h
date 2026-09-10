#pragma once
#include "BreakpointFitter.h"

namespace breakpoint_probe {
struct LikelihoodScores {
  double forward=0, backward=0, smoothed=0;
  double forwardQuadratic=0, backwardQuadratic=0, smoothedQuadratic=0;
  double forwardLogdet=0, backwardLogdet=0, smoothedLogdet=0;
  double hitPenalty=0, seedPenalty=0, processPenalty=0;
  int measurements=0, latentDimensions=0;
};
// Three evaluations of ONE frozen affine Gaussian model. This does not
// update a track or replace any native KalTest measurement update.
LikelihoodScores likelihoodScores(const breakpoint::FitResult& fit);
}
