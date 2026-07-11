#!/usr/bin/env python
"""Focused event-11 retained-lineage test with the ACTS/ATLAS BH model."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_lineage_smoothing_event11.py").read(),
    "run_gsf_lineage_smoothing_event11.py", "exec"))

gsf.BHModel = "ActsAtlas"
out.filename = "/tmp/gsf-lineage-acts-event11.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-acts-event11.root"
