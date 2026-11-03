# Honor ElectronOnly even when a neutral is collected

Deployment follow-up: this fix is now installed with the optional111 scanner;
see `2026-11-04-full-higgs-three-setup-scan.md`. The pending-install wording
below describes the original staged correction, not the current runtime.

The user clarified that ElectronOnly must constrain the electron only even
with a collected neutral. The unconditional override installed in6e505af was
incorrect; its complete implementation/testing evidence remains preserved in
`2026-11-04-automatic-resolved-sum110.md`. This is a routing fix, not a new option.

## Active control contract

ChargedEcalEnergyMode alone controls110, for ordinary and free-loss RTS:

| Mode | No collected neutral, one breakpoint | Collected neutral, one breakpoint |
|---|---|---|
| ElectronOnly | Charged energy -> electron | Charged energy -> electron |
| ElectronPhoton | Charged energy -> electron+photon | Charged+neutral energy -> electron+photon |

No breakpoint: always electron-only. 000/100 remain ECAL-free. 111 always
attempts the separate charged-electron and neutral-photon measurements, whatever
the110 mode. No Sum110 posterior is reused; each stage refits raw observations
from the original seed/prior. Missing/failure routing and early/delayed neutral
placement are unchanged. Passive neutral collection can supply the110 sum even
with the separate-neutral switch off, but only in ElectronPhoton mode.

Normal and HitScanAll controllers share a small pure routing predicate. The
shared sum assembler rejects an ElectronOnly state instead of silently changing
its mode. HitScanAll retains the original requested mode until each stage;111
explicitly selects electron-only charged energy. Legacy HitScan's deliberately
neutral-free scoring contract is unchanged. No geometry, ownership, fitter
algebra, optimizer, shared KF, PFA, diffuse or output-family change.

ElectronPhoton requires ElossOn=False; ElectronOnly regains its previous ability
to use ElossOn without being forced into the sum model. Maintained steering
already has ElossOn=False. Batch default ElectronPhoton is unchanged.
ResolvedEcalEnergyMode stays removed; no replacement selector introduced.

## Validation/deployment

Temporary runtime/evidence: `/tmp/bp-electron-only-routing-EHiDSX/`.
15 CTests and59 Python batch-helper tests passed, including every mode/breakpoint/
neutral routing combination and explicit rejection of an ElectronOnly sum.
All focused integrations passed:12 local event passes. ElectronPhoton seed1
events5,11,16,17,31,38 reproduce all6,024 branch-row values from the previous
default. ElectronOnly events5/31 reproduce2,008 values from the earlier
charged-only110/separate111 workflow, after adjusting only the requested-mode
label and omitting the already-retired requested resolved-mode field.
ElectronOnly110 measurement prediction+residual equals charged energy alone,
not charged+neutral; its sum-applied flag is false despite usable collected
neutrals. The two additional110-only runs match with the neutral switch off.
All separate111 fields are identical between the two current modes.

Full HitScanAll event5 scans passed in both modes (40 entries each): Photon
matches1,004 branch-row values from the prior default; Only matches1,004 from
the earlier separate workflow. Sum flags count nine sites for Photon, zero
for Only. All IP parameters/covariances, NLL/status/source data are preserved.
No online scan selection, bound change or acceptance repair.

Evidence: validate.py, validation.log and verbose Gaudi logs in the temporary
directory. Staged library SHA256:
`419dab584478850da799dfe45c554e0a7b448c70d6843cc7188172fc2d8c8494`.
No Condor jobs submitted; no existing tuple/card overwrite or remote operation.
The project-default installation is NOT changed by this correction yet;
installation awaits the requested quiet-window confirmation. The preceding
installed automatic-sum version still overrides ElectronOnly and must not be
mistaken for the validated staged correction.

The live focus is corrected in place. Prior defaults and numerical evidence
are retained in the automatic/optional resolved-sum records; active project laws
and the continuing scan-only contract are unchanged.
