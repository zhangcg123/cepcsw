#!/usr/bin/env python
"""Diagnostic only: retain split lineages for three hits before TopN pruning."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_lineage_smoothing_event11.py").read(),
    "run_gsf_lineage_smoothing_event11.py", "exec"))

gsf.ReductionMinHitsAfterSplit = 3
out.filename = "/tmp/gsf-lineage-delayed-event11.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-delayed-event11.root"
