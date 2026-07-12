# DD4hep material-transition resolution — 2026-07-11

This record preserves the material-ownership audit superseded when the active
focus moved to fitting the CEPC step-conditioned Bethe-Heitler model.

## Surface and crossed-cradle audit

In the matched 12-event seed-1 electron sample at 2.008 GeV and 85 degrees,
event 11 had 232 common outgoing transitions. Geant4 summed to 0.0737544 X0,
whereas the legacy current-surface GSF calculation summed to 0.0191777 X0, a
factor 3.85. TPC-to-TPC and VXD-to-VXD median Geant4/GSF ratios were 1.00006
and 1.0313; inter-detector/support transitions had median ratio 5.66. The hard
eBrem ITK transition was 0.00719995 X0 in Geant4 and 0.00161996 X0 in the
active surface estimate.

A GSF-local crossed-cradle diagnostic raised the event-11 total to 0.0634883
X0, but bounded and passive intervals alternated between over- and
under-counting, and the hard-eBrem transition remained low by a factor 1.85.
Intermediate active layers in the cradle scan were initially falsely counted;
excluding them and enforcing surface bounds did not resolve the missing
support and service material. The cradle representation is therefore retained
only as a diagnostic, not as authoritative transition material.

The historical loop also skipped the hit-0 outgoing transition, which was
0.000689961 X0 in matched Geant4 event 11.

## Resolution

The opt-in `MaterialPathMode="DD4hepBetweenSurfaces"` integrates DD4hep volume
material between each component's successive predicted measurement-surface
positions and processes hit 0 exactly once. For event 11 the DD4hep total was
0.0739544 X0, giving Geant4/DD4hep = 0.99730. The hard-eBrem transition was
0.00720455 X0, giving Geant4/DD4hep = 0.99936.

With this material mode, the provisional `Current` BH control completed all
234 hits with zero update rejection on events 11, 16, and 17. Its IP pT values
were 1.7936, 1.8148, and 1.5789 GeV, compared with LCIO 1.7934, 1.8118, and
1.5790 GeV. Correct material alone therefore did not provide truth recovery.

The ACTS/ATLAS implementation also contained an independent coefficient-order
error: ACTS stores polynomial coefficients highest order first. Correcting the
Horner evaluation made the model executable with DD4hep material; event 11
then retained 234/234 hits but gave pT 1.7933 GeV versus LCIO 1.7934 GeV. This
is implementation evidence only, not CEPC validation of the ATLAS mixture.
