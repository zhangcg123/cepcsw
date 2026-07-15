#!/usr/bin/env python
"""Focused forward validation of CEPC2GeV85StepConditioned on one event."""

import os

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py").read(),
    "run_gsf_reverse_template.py", "exec"))

gsf.BHModel = "CEPC2GeV85StepConditioned"
gsf.MaterialPathMode = "DD4hepBetweenSurfaces"
gsf.SelectedEventIndices = [int(os.getenv("GSF_SELECTED_EVENT_INDEX", "11"))]
verbose_dump = os.getenv("GSF_VERBOSE_DUMP", "1").lower() not in (
    "0", "false", "no")
gsf.VerboseDump = verbose_dump
gsf.VerboseSplitDump = verbose_dump
gsf.ComponentDebugDump = verbose_dump
gsf.ReverseFiltering = False
gsf.GaussianSumSmoothing = False
gsf.MaxComponents = 24
gsf.ReductionTargetComponents = 12
gsf.ElossOn = False
dsvc.input = os.getenv("GSF_INPUT_FILE", "tuples/trk-e--2.0-85-1.root")

out.filename = os.getenv(
    "GSF_OUTPUT_FILE", "/tmp/gsf-cepc2gev85-conditioned-event11.root")
flat.OutputFile = os.getenv(
    "GSF_FLAT_OUTPUT_FILE", "/tmp/gsf-flat-cepc2gev85-conditioned-event11.root")
