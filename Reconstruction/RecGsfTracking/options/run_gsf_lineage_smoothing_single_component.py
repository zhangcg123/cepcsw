#!/usr/bin/env python
"""One-component RTS control: event 11 with BH splitting disabled."""

exec(compile(
    open("Reconstruction/RecGsfTracking/options/run_gsf_lineage_smoothing_event11.py").read(),
    "run_gsf_lineage_smoothing_event11.py", "exec"))

gsf.ElectronHypothesis = False
gsf.MaxComponents = 1
gsf.ReductionTargetComponents = 1
out.filename = "/tmp/gsf-lineage-single-event11.root"
flat.OutputFile = "/tmp/gsf-flat-lineage-single-event11.root"
