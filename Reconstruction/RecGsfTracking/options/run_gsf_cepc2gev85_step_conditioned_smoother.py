#!/usr/bin/env python
"""Focused KL reduction-aware GSF smoothing for CEPC2GeV85StepConditioned."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/"
         "run_gsf_cepc2gev85_step_conditioned_event11.py").read(),
    "run_gsf_cepc2gev85_step_conditioned_event11.py", "exec"))

gsf.GaussianSumSmoothing = True
gsf.ReverseFiltering = False
gsf.MaterialIPExtrapolation = False
# Publish the Gaussian sum, including between-path covariance, rather than
# discarding it in favor of the single largest smoothed path.
gsf.GSFOutputMode = "WeightedMean"
