# ECAL focus handoff to material/BH consistency

Date: 2026-08-18

## Reason for the focus change

The default-off ECAL component-re-ranking prototype has reached a clear
boundary: it can improve some retained alternatives, especially hard losses
around transitions 3--6, but it does not repair components that are absent or
have negligible tracker posterior support. It also changes too many ordinary
GSF tracks that begin inside the 1% core. The user therefore paused ECAL
development and made material-path/Bethe-Heitler consistency the active
question.

This is a priority change, not a validation or rejection of calorimeter
information. The prototype remains default-off and paired; no fitted component
is changed and ordinary `GSFTracks` remains the tracker-only result.

## Preserved outgoing boundary

The broad paired screen contained 40,091 finite events. Of these, 35,681 were
topology-clear and 4,410 were the secondary-tracker-activity control. The clear
sample contained 14,924 no-eBrem, 16,638 light-eBrem, and 4,119 hard-eBrem
events under outgoing surface ownership.

Within the topology-clear sample, ECAL changed 150 branches: 106 improved and
44 worsened. Of 35 changed cases whose ordinary GSF residual started within
1%, only five improved and 30 worsened; 22 moved outside 1%, two outside 3%,
and none outside 10%. Central widths were essentially unchanged. Greater-than-
100% tails were reduced, but the largest ordinary overshoot (+42,661%) was
unchanged.

The exact selector gives ECAL at most a factor-20 relative score boost because
of the 0.05 likelihood floor. Successful focused competitors had tracker-score
ratios 0.989, 0.746, 0.194, and 0.0549. Several unrepaired extremes had ratios
of 0.00267 or much smaller. An available early-transition underestimate had a
truth-like component about 1265 times below the selected tracker score. A gate
threshold alone therefore cannot recover absent or negligible support.

## Deferred work

If ECAL work resumes, keep it default-off and paired. Diagnose the 44 worsened
topology-clear changes, especially the 30 that started inside the ordinary 1%
core, and audit matching and energy closure against the successful hard
transition-3--6 changes before proposing another selector. Freeze any proposal
before evaluating an independent broad sample and keep no/light/hard and the
secondary control separate.

The complete evidence remains in:

- `2026-08-17-default-off-ecal-component-constraint-prototype.md`;
- `2026-08-17-flat-tuple-paired-ecal-track-output.md`;
- `2026-08-17-expanded-overshoot-branch-choice-diagnosis.md`;
- `2026-08-17-broad-ecal-component-constraint-population-screen.md`;
- `2026-08-17-topology-clear-ecal-category-transition-screen.md`.
