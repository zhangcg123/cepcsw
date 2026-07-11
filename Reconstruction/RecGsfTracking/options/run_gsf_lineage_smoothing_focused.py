#!/usr/bin/env python
"""Verbose retained-lineage RTS validation for focused events 11, 16, 17."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_lineage_smoothing_event11.py").read(),
    "run_gsf_lineage_smoothing_event11.py", "exec"))

gsf.SelectedEventIndices = [11, 16, 17]
out.filename = "/tmp/gsf-lineage-focused.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-focused.root"
