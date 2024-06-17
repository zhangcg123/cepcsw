

This is about the temporary `TrueMuonTagAlg`.

The `TrueMuonTagAlg` is implemented in:

`
Reconstruction/RecAssociationMaker/src/TrueMuonTagAlg.h|cpp
`

refering to the `TrackParticleRelationAlg` implemented in the same folder.

It first matches the `FullLDCTrack` with a `MCParticle`.
If the `MCParticle` is a muon, according to a true efficiency `MuonTagEfficiency`,
randomly flag it as having a muon Tag or not,
separately for Barrel and Endcap.

The flag is stored in the `edm4hep::MutableTrack::type()`,
following the `lcio::ILDDetID::` numbers, which is defined in:
[LCIO-02-20/src/cpp/src/UTIL/ILDConf.cc](https://github.com/iLCSoft/LCIO/blob/v02-20/src/cpp/src/UTIL/ILDConf.cc)


An example configure is shown in `tracking_trueMuonTag.py`, e.g.:

```
from Configurables import TrueMuonTagAlg
tmt = TrueMuonTagAlg("TrueMuonTag")
tmt.MCParticleCollection = "MCParticle"
tmt.TrackList = ["CompleteTracks"]
tmt.MuonTagEfficiency = 0.95 # muon true tag efficiency, default is 1.0 (100%)
tmt.MuonDetTanTheta = 1.2 # muon det barrel/endcap separation tan(theta)
#tmt.OutputLevel = DEBUG
```

In this example, it adds the muon Tag in the `edm4hep::MutableTrack::type()` for each track in
"CompleteTracks" collection (which is a `FullLDCTrack` collection)
and output it to be "CompleteTracksWithMuonTag" in your output `lcio` root files.

To use it in your analysis codes, one can do the following, either in your cpp codes in CEPCSW,
or in your `ROOT` analysis codes.

Example 1, for CEPCSW Algorithms:

Add the following in your header file:
```
#include <UTIL/ILDConf.h>
```

Do the following in your cpp file, suppose `track` is the track you get from the
"CompleteTracksWithMuonTag" collection:
```
bool hasMuonTag_MuonBarrel = ( ( track.type() >> lcio:ILDDetID:YOKE ) % 2 == 1 );
bool hasMuonTag_MuonEndcap = ( ( track.type() >> lcio:ILDDetID:YOKE\_ENDCAP ) % 2 == 1 );
```

Example 2, for ROOT analysis .C macros, suppose `events` is the `TTree` in your output root
file storing the events.

```
events.Draw("((CompleteTracksWithMuonTag.type>>27) % 2)>>hHasMuonTag_MuonBarrel");
events.Draw("((CompleteTracksWithMuonTag.type>>31) % 2)>>hHasMuonTag_MuonEndcap");
```

This will draw one histogram for has muon tag in barrel or in endcap.
in case a track has muon tag, it is one entry at 1 in this histogram.
And the numbers 27 and 31 are the numbers defined for `lcio:ILDDetID:YOKE`
and `lcio:ILDDetID:YOKE_ENDCAP` in `UTIL/ILDConf.h`, respectively.



