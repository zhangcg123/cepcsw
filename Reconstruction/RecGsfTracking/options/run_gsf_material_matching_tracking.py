#!/usr/bin/env python
"""Run standard TDR tracking for a small G4/GSF material-matching sample."""

import os

exec(compile(
    open("Detector/DetCRD/scripts/TDR_o1_v01/tracking.py").read(),
    "Detector/DetCRD/scripts/TDR_o1_v01/tracking.py", "exec"))

dsvc.input = os.environ["GSF_MATCH_SIM_INPUT"]
out.filename = os.getenv("GSF_MATCH_TRACK_OUTPUT", "/tmp/gsf-match-tracks.root")
ApplicationMgr().EvtMax = int(os.getenv("GSF_MATCH_EVTMAX", "20"))
