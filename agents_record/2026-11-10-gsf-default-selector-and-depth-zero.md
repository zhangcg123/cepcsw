# Normal GSF selector installation and maintained-card defaults (2026-11-10)

The user explicitly requested that the default GSF run card use look-ahead
depth zero and that the two-electron selector be installed and run by default.
The preceding fresh no-running-jobs confirmation remains applicable; no user
batch was submitted during this work. Stay on breakpoint-delayed-neutral.

## Scope and defaults

Promote the existing private selector implementation (799c0f6) into the normal
RecGsfTracking package, including its flat-tuple mapping/status fields,
integration checker, shared-selector unit target and package build dependency.
The standalone TruthElectronSelector source is compiled from RecBreakpoint;
it is unchanged, and no RecBreakpoint plugin dependency is introduced.

The maintained DumpGsfTrks/gsf.py.bk now sets:

```python
gsf.SelectTruthProductionElectrons = True
if gsf.SelectTruthProductionElectrons:
    podioinput.collections += ["CompleteTracksParticleAssociation"]
gsf.InwardLookaheadDepth = 0
```

Both base and reverse-branch depth assignments are zero. Selection is enabled
before its conditional association input is evaluated. Bare-algorithm and
unsteered active reverse-template selector defaults remain false, preserving
other cards' explicit all-track behavior. Single-particle/gun cards must set
the maintained card's gate false before the conditional reader block.
No other effective maintained reverse physics setting changes: fresh inward
seed -1, both BH gates on, beam on, ECAL/oracle off, MaxComponents10,
LocalMeasurement and WARNING ordinary output remain in the working card.
Earlier dirty BH/beam/logging card changes are preserved separately from the
task-only Git checkpoint; the user-requested depth2->0 change supersedes the
earlier uncommitted depth2 override. Prepared/frozen older cards are untouched.

Selection follows the same generator/association contract as breakpoint:
unique stable electron/positron outside Higgs ancestry, requiring a generator
Higgs record, all-parent ancestry traversal, unique reconstructed matching and
no all-track fallback on ambiguity or missing inputs. One uniquely
reconstructed partner can still run. The gate precedes all expensive per-track
fit workflows and preserves original input indices. Five automatic EDM/flat
maps/status fields retain selection and successful output indexing. The truth
oracle/passive recorder index0 is not retargeted. Legacy scalar tuple fields
still do not represent both selected electrons.

The detached private source/build/install and prior comparison artifacts are
preserved. No shared tracking/simulation source changes, truth-hook fixes,
HitScan restoration or production submissions are part of this promotion.

## Dedicated option audit

The required gsf_selection_option_audit sub-agent performed a read-only full
audit. All44 custom controls are documented and their compiled defaults match
the README. The maintained card explicitly steers43, deliberately inheriting
only RecordTruthMaterialIntervals=True. Selector/card/template distinctions
and all reverse initialization compatibility requirements pass. Both READMEs
now distinguish truth-assisted track selection from truth-independent normal
fit dynamics and correctly document maintained ElossOn=False versus the
active reverse template's True. Look-ahead depth0 is synchronized throughout.

## Validation and installation

Artifacts: build.gsf-selection-dev/validation/gsf-card-defaults-20261110/.
Native inputs are exclusively the new production's rec-higgs-4.root.
The private reference uses the new maintained card settings, depth0, selected
event0 and comprehensive verbose component dumps; it selects/publishes original
tracks9/18. The promoted normal library is checked on the same focused event,
then source events11,16,17. Numerical reference is the same-settings private
run, not the older depth2 comparison. No population-performance claim follows.

Both RecGsfTracking and RecGsfFlatTuple build successfully in the normal
configured tree. The reused selector C++ unit test passes (ancestry, ambiguity,
association aggregation and failure cases). The required independent option
surface audit passes. A full CMake configuration refresh was performed after
copying the older-mtime private build file to regenerate selector include/source
dependencies; no shared-package source changes were needed.

The focused promoted-library run exits0, selects/publishes only original
tracks9/18, and EXACTLY matches the same-settings private reference in all255
flat-tuple branches across1 row. The follow-up run reads18 source entries and
fits only events11,16,17: selected/published originals are [2,8], [3,5], [22,23].
All8 selected tracks across the4 tested events publish; the integration checker
confirms that output maps, all lineage and all final components are restricted
to the selected pair, indices are consistent and component weights normalize.
All native checks use comprehensive component dumps. Skipped events have
status-5 and empty output/lineage/component maps.

The normal package-only install is complete. Installed tracking and flat-tuple
library hashes equal the tested staging copies. Fresh standard setup loads
SelectTruthProductionElectrons from the normal installed configuration, with
bare defaultFalse and compiled look-ahead0; the maintained card explicitly sets
selectionTrue/depth0. RecBreakpoint and RecGsfSimHitTuple library hashes remain
unchanged. Neither private source nor private installed libraries was replaced.
The outgoing complete AGENTS.md is preserved in
2026-11-10-agents-before-gsf-default-promotion.md; global instructions/laws are
retained byte-for-byte and the new focus supersedes the private-only status.
No prepared/frozen cards or user jobs were changed or submitted. New generated
cards inherit the requested defaults; older frozen cards require regeneration.
