#!/usr/bin/env python
"""Focused KL reduction-aware GSF smoothing for CEPC2GeV85StepConditioned."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/"
         "run_gsf_cepc2gev85_step_conditioned_event11.py").read(),
    "run_gsf_cepc2gev85_step_conditioned_event11.py", "exec"))

gsf.GaussianSumSmoothing = True
gsf.ReverseFiltering = False
gsf.MaterialIPExtrapolation = False
# Both endpoint views are published: GSFTracks carries BestBranch and
# GSFTracksWeightedMean carries the moment-matched Gaussian sum.
