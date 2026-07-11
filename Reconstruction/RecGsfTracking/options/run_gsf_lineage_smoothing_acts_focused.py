#!/usr/bin/env python
"""Focused 11/16/17 retained-lineage test with the ACTS/ATLAS BH model."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_lineage_smoothing_focused.py").read(),
    "run_gsf_lineage_smoothing_focused.py", "exec"))

gsf.BHModel = "ActsAtlas"
out.filename = "/tmp/gsf-lineage-acts-focused.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-acts-focused.root"
