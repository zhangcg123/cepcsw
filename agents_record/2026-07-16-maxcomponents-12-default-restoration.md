# MaxComponents=12 default restoration

Date: 2026-07-16

The user explicitly replaced the active `MaxComponents=24` baseline with
`MaxComponents=12`. This changes the C++ property, package README, standard
electron and test steering, reverse-template environment fallback, and focused
event-11 helper. Explicit 12-versus-24 analysis scripts remain unchanged.

The outgoing default was 24. Its comprehensive default-helper checks on light
events 284/1 and 404/8 matched the earlier explicit 24-component outputs at
tuple precision. The direct comprehensive-dump 12-versus-24 comparison on the
30 positive-LCIO-amplification candidates improved 10 events, worsened 16,
left four unchanged, and increased the count above +0.25-point amplification
from 21 to 26. In the 13 low-amplification controls, only one improved and nine
worsened because capacity 12 reactivated radiative selections that capacity 24
left on identity. Full provenance remains in
`agents_record/2026-07-14-positive-lcio-amplification-maxcomponents12-comparison.md`
and `agents_record/2026-07-13-maxcomponents-24-default-adoption.md`.

Therefore the restoration to 12 is an explicit working-baseline decision, not
evidence that 12 is physics-superior. New comparisons must state the component
capacity rather than assuming historical 24-component outputs are the default.
