#!/usr/bin/env python
"""Focused retained-lineage RTS smoothing for CEPC2GeV85StepConditioned."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/"
         "run_gsf_cepc2gev85_step_conditioned_event11.py").read(),
    "run_gsf_cepc2gev85_step_conditioned_event11.py", "exec"))

# RTS smoothing requires real, unmerged process lineages.
gsf.ReductionMode = "TopN"
gsf.RetainedLineageSmoothing = True
gsf.ReverseFiltering = False
gsf.MaterialIPExtrapolation = False
