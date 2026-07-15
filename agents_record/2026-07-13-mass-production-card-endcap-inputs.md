# Mass-production GSF card: explicit endcap inputs

Date: 2026-07-13

The user requested explicit endcap tracker collections in the active batch
template `DumpGsfTrks/gsf_reverse_new.py.bk`. The `PodioInput` collection list
now includes ITK/OTK endcap SimTrackerHits, reconstructed TrackerHits, and
tracker-hit associations. `RecGsfFlatTuple.HitCollectionNames` now includes
the ITK and OTK endcap reconstructed-hit collections so its auxiliary
`all_hit_*` dump covers them as well.

The physics configuration was not changed: `MaxComponents=24`,
`MaterialPathMode="DD4hepBetweenSurfaces"`, reverse `BestBranch`, and the CEPC
2 GeV/85-degree step-conditioned BH model remain selected. The template passes
Python syntax compilation. All six newly explicit collections are present in
representative `tuples285/trk-e--2.0-85-1.root` and
`tuples1020/trk-e--10.0-20-1.root` inputs.
