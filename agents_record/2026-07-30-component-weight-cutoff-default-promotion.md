# Component-weight cutoff default promotion

Date: 2026-07-30

The user promoted `ComponentWeightCutoff=1e-4` from the maintained
`DumpGsfTrks/gsf.py.bk` comparison setting to the compiled and active reverse
default. The superseded compiled and reverse-template default was `1e-8`.

The synchronized configuration surface is:

- `Reconstruction/RecGsfTracking/src/GsfAlgorithm.h`: compiled default `1e-4`;
- `Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py`: active
  reverse value `1e-4`;
- `DumpGsfTrks/gsf.py.bk`: explicit maintained value `1e-4`;
- `Reconstruction/RecGsfTracking/README.md`: compiled/active reference updated;
- `DumpGsfTrks/README.md`: the former intentional mismatch removed;
- `AGENTS.md`: active baseline updated.

This is a component-population policy change, not Bethe-Heitler or GSF
validation. It removes normalized target-measurement posterior components more
aggressively while retaining the largest component and, when enabled, the
protected identity lineage. Mechanical and physics validation must still
follow the focused-event and population gates in `AGENTS.md`.

The `RecGsfTracking` target built successfully in the configured EL9/LCG 105
tree after the promotion. The build emitted existing compiler/ROOT dictionary
warnings but returned success. No focused runtime or same-code population A/B
was performed: the expected local exact-pair input
`/tmp/gsf-match-tracks.root` was absent, and the historical records already
note that its last available copy ended after event 11. Events 11, 16, and 17
therefore remain a required validation gate before making a performance claim
for the promoted default.
