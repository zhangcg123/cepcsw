# MaterialPathMode direction-symmetry correction

Date: 2026-08-18

## Defect

The outward GSF material step selected its calculator through
`MaterialPathMode`, but the reverse multi-component refit bypassed that
property and always called the DD4hep between-surfaces integrator. Consequently
the nominal `CurrentSurface` production card actually combined current-surface
material outward with DD4hep interval material inward.

This was a mechanical dispatch defect, not evidence that either material model
is validated. It also means reverse outputs made before this correction are not
same-code comparable with corrected outputs.

## Correction

The reverse material dispatch now uses the same normalized
`MaterialPathMode` choice as the forward filter:

- `CurrentSurface`: both directions use material owned by the inner/outgoing
  measurement surface. During reverse propagation, its incidence is evaluated
  at the reverse component's crossing of that target surface before the target
  measurement update.
- `DD4hepBetweenSurfaces`: both directions integrate DD4hep material over the
  complete interval between the two bounding measurement surfaces.

The reverse verbose component dump now records the configured mode and the
effective `t/X0`. The property documentation in
`Reconstruction/RecGsfTracking/README.md` and its Gaudi description were
updated. No property name, accepted value, or default changed;
`DumpGsfTrks/gsf.py.bk` already explicitly steers
`MaterialPathMode = "CurrentSurface"` and required no synchronization edit.

## Mechanical verification

The modified `RecGsfTracking` target built and installed successfully in the
EL9/LCG 105 development build.

A focused verbose run used seed 216, entry 0 from
`rec-e--2.0-85-216.root`, once per mode, with all generated ROOT files and the
DD4hep log kept under `/tmp`:

- `CurrentSurface` completed one fitted track with all displayed reverse paths
  labeled `mode=CurrentSurface`. Representative TPC paths were about
  `4.25e-5 t/X0`; the inner surface path was about `1.60e-3 t/X0`. The reverse
  refit made one BH split. At hits 1 and 2 the forward and reverse normal
  thicknesses were identical (`4.24995619e-5 t/X0`), with the incidence-
  projected paths agreeing at the few-`1e-4` relative level.
- `DD4hepBetweenSurfaces` completed one fitted track with all displayed reverse
  paths labeled `mode=DD4hepBetweenSurfaces`. Representative inner intervals
  were about `0.0249`, `0.183`, and `0.032--0.0459 t/X0`. The reverse refit made
  29 BH splits. On the first inward interval, before reverse BH evolution
  changed the component trajectories, the corresponding forward and reverse
  paths agreed (`0.03197896` versus `0.03197973 t/X0`). Later intervals need
  not be numerically reciprocal because they are evaluated on different,
  repeatedly split component trajectories.

As a direction sanity check, forcing the outward (`+1`) crossing selector in
the reverse DD4hep calculation made the tested inward TPC crossings invalid;
the explicit inward (`-1`) selector is therefore retained.

The deliberately large behavioral difference confirms that the property now
selects the reverse calculator instead of the old hard-coded DD4hep path. This
is a dispatch and execution validation only; it does not promote the DD4hep
mode or establish physics performance for the corrected current-surface
baseline.

## Comparison boundary

The memory-leak tag remains useful provenance for that repair, but no longer
identifies the exact material behavior of the current reverse baseline. All
performance claims involving the reverse workflow must use newly generated
corrected outputs, and final comparisons remain subject to the project's
same-code direct A/B rule.
