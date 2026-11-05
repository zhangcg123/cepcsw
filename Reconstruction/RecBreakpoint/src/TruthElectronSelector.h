#ifndef RECBREAKPOINT_TRUTHELECTRONSELECTOR_H
#define RECBREAKPOINT_TRUTHELECTRONSELECTOR_H

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include <string>
#include <vector>

namespace breakpoint {
/// Truth-assisted eeH study selection, not reconstructed electron identification.
class TruthElectronSelector {
public:
  struct Result {
    // 0 disabled, 1 both tracks, 2 one track, -1 no Higgs, -2 nonunique truth
    // pair, -3 neither track usable, -4 invalid ancestry/input.
    int status = 0;
    std::string error;
    // Ordered e-, e+; track indices are ORIGINAL input ordinals, not new IDs.
    std::vector<int> mcIndices{-1, -1}, trackIndices{-1, -1}, candidateCounts{0, 0};
    bool accepts(int inputTrackIndex) const;
  };

  Result select(const edm4hep::MCParticleCollection& particles,
                const edm4hep::TrackCollection& tracks,
                const edm4hep::MCRecoTrackParticleAssociationCollection& associations) const;
};
} // namespace breakpoint
#endif
