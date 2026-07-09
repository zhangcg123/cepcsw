---
name: 2026-07-07-temp-gsf-electron-refit-crash
metadata:
  type: resolved-handoff
  date: 2026-07-07
  status: resolved
---

# Resolved Handoff: Electron GSF Refit Crash

## Context

The new material-step tuples look OK. The muon material-step ROOT file now has endpoint R around 2.0-2.2 m when using `TrackerOnly=False`, so the previous ~235 mm endpoint issue was from tracker-only recording.

The resolved issue was the electron GSF refit chain. The relevant log is:

```text
outlog/trk_e-_1.0_85_1.out
outlog/trk_e-_1.0_85_1.err
```

Despite the filename prefix `trk_`, this log includes the GSF stage because `dump_gsftrk.sh` runs sim/trk/gsf in sequence.

## Observed Failure

The electron GSF stage crashed after 118 event-level `Fitted:` messages with:

```text
TKalTrack::At ERROR index 1 out of bounds (size: 1, this: ...)
TUnixSystem::DispatchSignals FATAL segmentation violation
#5 RecGsfTracking::execute()
```

The output ROOT files from that crashed run were not finalized correctly:

```text
gsf-e--1.0-85-1.root       no keys
gsf_flat-e--1.0-85-1.root  no keys
```

## Diagnosis

The crash is not just an IP-extrapolation guard problem. The root cause is earlier in `RecGsfTracking::execute()`:

```cpp
if (comp->kaltrack->AddAndFilter(*st)) {
  accepted.push_back(comp);
} else {
  delete st;
  comp->weight *= 1e-6;
  if (comp->weight > 1e-30) accepted.push_back(comp);
  else delete comp;
}
```

A failed component was kept alive with a tiny weight but with a stale Kalman track. If this happens before any real hit is added, the component can still have only the dummy seed site. Later the best component may be selected and passed to:

```cpp
comp->kaltrack->At(1)
```

inside IP extrapolation, causing the out-of-bounds crash.

## Code Patch Applied

File patched:

```text
Reconstruction/RecGsfTracking/src/GsfAlgorithm.cpp
```

Changes:

1. Added event-index info log at the start of `RecGsfTracking::execute()`:

```cpp
info() << "GSF event index " << (m_nEvt - 1)
       << " (event count " << m_nEvt << ")" << endmsg;
```

2. Changed failed `AddAndFilter` handling to reject/delete failed components instead of keeping stale components:

```cpp
} else {
  delete st;
  delete comp;
}
```

3. If all components fail at a hit, warn and stop fitting that track without writing an invalid output track:

```cpp
warning() << boost::format("GSF event index %d track %d: all components rejected at hit %d (r=%.1f mm); no GSF output track")
...
comps.clear();
break;
```

4. Best-component selection now requires a component with at least one real filtered hit:

```cpp
if (!comps[i]->kaltrack || comps[i]->kaltrack->GetEntriesFast() <= 1) continue;
```

This is intended to solve the actual stale-component problem, not merely hide the crash.

## Resolution Status

Resolved as of 2026-07-07. Do **not** treat this file as the active next task.

The patched `RecGsfTracking` library was installed and loaded by the normal setup-wrapper run. Evidence checked after repair:

```text
InstallArea/x86_64-el9-gcc11-opt/lib/libRecGsfTracking.so  timestamp 2026-07-07 04:53
build.105.0.0.x86_64-el9-gcc11-opt/lib/libRecGsfTracking.so timestamp 2026-07-07 04:53
```

Validation command used:

```bash
source setup.sh
./run.sh DumpGsfTrks/rungsf-e--1.0-85-1.py \
  > outlog/gsf_e-_1.0_85_1.fixed.out \
  2> outlog/gsf_e-_1.0_85_1.fixed.err
```

Validation result:

```text
outlog/gsf_e-_1.0_85_1.fixed.out contains the new `GSF event index ...` log line
Processed 100 events
RecGsfFlatTuple wrote 100 entries to gsf_flat-e--1.0-85-1.root
No ERROR/FATAL/segmentation message found in the inspected final grep
```

Output files were finalized and attachable by ROOT:

```text
gsf-e--1.0-85-1.root       11M, timestamp 2026-07-07 20:06
gsf_flat-e--1.0-85-1.root  1.2M, timestamp 2026-07-07 20:06
```

The old crash mechanism remains documented below for provenance, but the actionable next step is no longer crash repair. Continue from `agents_record/current-stage-and-todos.md`: true G4-step analysis, larger electron samples, and CEPC-specific BH mixture fitting.

## Other Current Edits To Remember

- `DumpGsfTrks/gsf.py.bk` has INFO output enabled.
- Generated `rungsf-*.py` files should generally be ignored unless regenerated from templates.
- Remote SSH session should be kept open when possible.
