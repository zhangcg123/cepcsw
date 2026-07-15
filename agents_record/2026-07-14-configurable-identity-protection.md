# Configurable identity-lineage protection

Date: 2026-07-14

## Motivation

The unprotected `TopN`, `MaxComponents=12` survey showed that pure weight-rank
pruning can delete the exact no-radiation lineage. Identity was absent from the
final reverse mixture in 9/30 positive-LCIO-amplified events, and seed 193/event
9 plus seed 233/event 4 exhausted every reverse component. This made their
fallback outputs invalid comparisons. The user requested that the explicit
identity protection be configurable so protected and purely statistical
reductions can be compared with otherwise identical settings.

## Implementation and semantics

`RecGsfTracking` now exposes the Gaudi boolean property
`ProtectIdentityLineage`, default `true`.

- Low-weight cutoff preserves exact `noRadiationLineage` components only when
  the property is enabled. The largest component is still retained regardless.
- KL reduction skips identity/radiative cross-lineage merges only when the
  property is enabled and the target is greater than one. Disabling it restores
  unrestricted minimum-KL merging.
- TopN reduction, when enabled and the target is greater than one, retains the
  highest-weight exact identity. If identity would fall below the rank cutoff,
  it replaces the lowest-ranked retained component. Disabling protection
  restores strict weight-rank TopN pruning.
- The same setting is applied in forward and reverse filtering.
- `run_gsf_reverse_template.py` exposes the switch as
  `GSF_PROTECT_IDENTITY_LINEAGE`; accepted true values are `1`, `true`, and
  `yes`, and the default is `1`.

The implementation is confined to `Reconstruction/RecGsfTracking` and does
not change the active `MaxComponents=24`, KL, aggregate-weight default.

## Build and focused validation

The configured EL9/LCG 105 `RecGsfTracking` target built and the build tree was
installed successfully.

A comprehensive component-dump A/B used the same seed 193/event 9 input and
identical `ReductionMode=TopN`, `MaxComponents=12` settings:

- Protection disabled: configuration reports
  `protectIdentityLineage=0`; the prior failure is reproduced with
  `finalComps=0`, 2266 accepted component updates, one rejected update, five
  splits, three reductions, and no reverse IP output.
- Protection enabled: configuration reports
  `protectIdentityLineage=1`; each reduction reserves the final retained slot
  for the exact identity when it is otherwise below rank 12. The run finishes
  with `finalComps=12`, 2795 accepted component updates, one rejected update,
  eight splits, eight reductions, and a finite reverse IP output with
  `pT=2.0213 GeV`.

Both Gaudi jobs terminate successfully. The paired logs are temporary outputs:

- `/tmp/gsf-identity-protection-topn12-off.log`
- `/tmp/gsf-identity-protection-topn12-on.log`

This validates configurability and prevents the known TopN component-exhaustion
failure. It is not evidence that protected TopN-12 improves the physics sample;
the active physics baseline remains protected KL-24, and any proposal to use
TopN still requires the ordered population and hard-loss controls in
`AGENTS.md`.
