#ifndef TrackerHitHelper_H
#define TrackerHitHelper_H
#include <optional>
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/SimTrackerHit.h"

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#include "edm4hep/TrackerHit3D.h"
#include "edm4hep/TrackerHitPlane.h"
#else
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif

#include "DDSegmentation/Segmentation.h"
#include "DetSegmentation/GridDriftChamber.h"
#include <array>

//namespace dd4hep {
//         class Detector;
//         namespace DDSegmentation{
//             class GridDriftChamber;
//         }
//}

namespace CEPC{
  std::array<float, 6> GetCovMatrix(edm4hep::TrackerHit& hit, bool useSpacePointerBuilderMethod = false);
  float                GetResolutionRPhi(edm4hep::TrackerHit& hit);
  float                GetResolutionZ(edm4hep::TrackerHit& hit);
  std::array<float, 6> ConvertToCovXYZ(float dU, float thetaU, float phiU, float dV, float thetaV, float phiV, bool useSpacePointBuilderMethod = false);


#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
#else
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
#endif
    
    
  const edm4hep::SimTrackerHit getAssoClosestSimTrackerHit(
        const CEPCSWTrackerHitSimTrackerHitLinkCollection* assoHits,
        const edm4hep::TrackerHit trackerHit,
        const dd4hep::DDSegmentation::GridDriftChamber* segmentation,
        int docaMehtod);

 std::optional<edm4hep::TrackerHit> getAssoTrackerHit(
          const CEPCSWTrackerHitSimTrackerHitLinkCollection* assoHits,
          edm4hep::SimTrackerHit simTrackerHit);

  const edm4hep::SimTrackerHit getAssoSimTrackerHit(
          const CEPCSWTrackerHitSimTrackerHitLinkCollection* assoHits,
          edm4hep::TrackerHit trackerHit);
}

#endif
