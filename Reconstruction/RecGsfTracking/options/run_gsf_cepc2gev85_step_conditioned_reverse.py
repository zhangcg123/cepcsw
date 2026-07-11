#!/usr/bin/env python
"""Focused ACTS-style backward GSF for CEPC2GeV85StepConditioned."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/"
         "run_gsf_cepc2gev85_step_conditioned_event11.py").read(),
    "run_gsf_cepc2gev85_step_conditioned_event11.py", "exec"))

gsf.ReductionMode = "KL"
gsf.RetainedLineageSmoothing = False
gsf.ReverseFiltering = True
gsf.MaterialIPExtrapolation = False
