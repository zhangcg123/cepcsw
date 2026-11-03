# Reconstructed TPC row-0 candidate in HitScanAll

The user requested the point corresponding to effective TPC-inner-wall
ownership be included as a regular location-scan candidate. This extends
HitScanAll only; legacy ranked HitScan remains silicon-only.

## Contract

- Regular candidates are every reconstructed VXD/ITK hit plus the first exact
  system4/layer0 reconstructed hit, in ordered-track-hit order. Exclude the
  final hit as before. Missing row0 never substitutes a different TPC row.
- Candidate construction reads reconstructed cell IDs, not truth. The TPC
  digitizer writes its zero-based row index into the reconstructed layer field
  (`TPCDigiAlg.cpp`, layer assignment from `seed_hit->getRowIndex()`).
- Preserve existing kind values:0 null,1 silicon,2 truth-only. New kind3 marks
  the regular TPC row-0 candidate. Offline ranking must include kinds1 and3,
  never silently promote kind2. A truth owner at the same hit reuses that
  candidate and does not add a duplicate or change candidate order.
- Every point still records000/100/110/111. Optimizer, likelihood, constraints,
  failure inheritance, no-breakpoint control, truth ownership, fit algebra,
  and installed defaults are unchanged. No online location ranking is added.

The200-job/10k-event campaign completed before this extension. Its seven plots
were refreshed from all200 files:4936 eligible-truth hard tracks,2434 hard
silicon truth-site tracks,2358 common all-setup silicon tracks. Existing
tuples are not rewritten or treated as a truth-independent row0 scan. Rerun
the breakpoint stage after deployment; sim/rec need not be regenerated.

## Validation and deployment status

- Standalone candidate/selector test passed, including row0 present, missing,
  repeated, outermost-only, truth reuse and misaligned detector/layer vectors.
- Full RecBreakpoint build succeeded;14 CTests and56 batch-helper tests passed.
- Focused Gaudi regressions and scan checks completed under
  `/tmp/bp-scan-tpc-row0-7RNVnE`. Original Truth-mode events11/16/17 reproduce
  all2970 checked branch-row values exactly, NaN-aware, across both trees.
- Scan events5/17/38 each contain a regular kind3 TPC-row0 point with all four
  setups, valid finite results and the unchanged NLL decomposition. For
  events5/38, the earlier silicon/null fits retain every checked NLL, pT/error, optimized
  prior, posterior b/variance, mask, dimensions, source and fit status exactly.
- Event17's effective TPC-inner-wall truth owner at hit9 reuses that same
  regular point, not an extra truth-only point. All four results match the
  independently run fixed truth-location refits exactly in pT/error and
  within1e-7 in NLL. No truth loss amount is used by the scan.
- The user confirmed no active breakpoint jobs and authorized installation
  after validation. Installed ONLY the RecBreakpoint package library,
  configurable and discovery metadata using its package cmake_install.cmake.
  The default-environment EvtMax0 probe loads the frozen Higgs scan card and
  terminates successfully with an isolated /tmp output. It is a deployment
  check, not a physics validation. No /tmp runtime override is used by jobs.
- Installed/tested library SHA256:
  `5b72f16565ba033168defbda1f2eb2f8cb379cafc4fa85861cbc8183d43fab1f`.
  Previous installed SHA256:
  `f16257eb68ad4eb40aa8c6e3dff3b91c20296fd5cd21e9ea2f8f15722c7b9383`;
  the previous library is also backed up in the temporary test area.
- No batch, branch, remote, PFA, shared KF or GSF changes. Build retained the
  pre-existing clock-skew and external-header warnings; no workaround was
  added to shared code.

Focused row0 checks (pT in GeV, requested setups000/100/110/111):

| Event | Row0 hit | Total points | pT000 | pT100 | pT110 | pT111 |
|---|---:|---:|---:|---:|---:|---:|
| 1:5 | 8 | 10 | 11.297486709 | 11.322731062 | 11.318446054 | 13.311293675 |
| 1:17 | 9 | 11 | 35.559667603 | 35.520663605 | 35.460443356 | 35.460443356 |
| 1:38 | 9 | 11 | 12.971758792 | 12.970799164 | 13.014468753 | 13.014468753 |

Event5 has actual masks0/1/3/7. Events17/38 lack a collected neutral and have
masks0/1/3/3:111 inherits110. These are mechanical checks, not a claim that
row0 is their preferred location or that momentum/identification performance
improves. The scanner still performs no online ranking. No new batch cards
were generated and no stored scan tuples/plots were rewritten by this change.

## Memory preservation

Complete pre-edit AGENTS is preserved in
`2026-11-04-agents-before-tpc-row0-scan.md`. Section1 (global status, laws,
scope and commands) is unchanged. Section2 is updated in place; its outgoing
text is preserved completely in that snapshot. No history directory moved
and no history file was deleted.
