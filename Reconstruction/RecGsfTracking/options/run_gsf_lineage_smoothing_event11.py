#!/usr/bin/env python
"""Focused retained-lineage RTS smoothing validation for hard-loss event 11."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py").read(),
    "run_gsf_reverse_template.py", "exec"))

gsf.SelectedEventIndices = [11]
gsf.VerboseDump = True
gsf.VerboseSplitDump = True
gsf.ComponentDebugDump = True
gsf.ReductionMode = "TopN"
gsf.ReverseFiltering = False
gsf.RetainedLineageSmoothing = True
gsf.MaterialIPExtrapolation = False
dsvc.input = "tuples/trk-e--2.0-85-1.root"

out.filename = "/tmp/gsf-lineage-event11.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-event11.root"
