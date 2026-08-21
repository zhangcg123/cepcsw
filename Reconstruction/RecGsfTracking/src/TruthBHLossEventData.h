#ifndef RecGsfTracking_TruthBHLossEventData_h
#define RecGsfTracking_TruthBHLossEventData_h

#include "GsfTruthEventData/G4MaterialStepCollection.h"
#include "GsfTruthEventData/SimTrackerHitG4StepLinkCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/TrackerHit.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct TruthMaterialIntervalMatch {
  int firstStepNumber = -1;
  int lastStepNumber = -1;
  double startHookFraction = -1.0;
  double endHookFraction = -1.0;
  edm4hep::Vector3d startPosition{};
  edm4hep::Vector3d endPosition{};
  int stepCount = 0;
  double truthTX0 = 0.0;
  double momentumBefore = 0.0;
  double ebremLoss = 0.0;
  double retainedFraction = 1.0;
};

struct TruthBHLossEventDataMatch {
  std::vector<double> retainedFractions;
  std::vector<TruthMaterialIntervalMatch> materialIntervals;
  double maxEndpointDistance = 0.0;
  int g4TrackID = -1;
};

/// Event-local, relation-driven Geant4 truth lookup for the default-off BH
/// oracle. The lookup chain is
/// TrackerHit -> MCRecoTrackerAssociation -> SimTrackerHit -> exact G4 hook.
/// No spatial search is used to choose a truth hit or a Geant4 step.
class TruthBHLossEventData {
public:
  bool prepare(
      const gsftruth::G4MaterialStepCollection* steps,
      const gsftruth::SimTrackerHitG4StepLinkCollection* links,
      const std::vector<const edm4hep::MCRecoTrackerAssociationCollection*>&
          associations,
      std::string& error);

  bool matchTrack(const std::vector<edm4hep::TrackerHit>& orderedHits,
                  double maxEndpointDistance,
                  bool collectMaterialIntervals,
                  TruthBHLossEventDataMatch& match,
                  std::string& error) const;

private:
  using ObjectKey = std::pair<int, int>;
  using StepKey = std::pair<int, int>;

  struct StepRecord {
    int trackID = -1;
    int parentID = -1;
    int stepNumber = -1;
    int pdg = 0;
    int processSubtype = 0;
    edm4hep::Vector3d prePosition{};
    edm4hep::Vector3d postPosition{};
    edm4hep::Vector3f preMomentum{};
    edm4hep::Vector3f postMomentum{};
    double momentumLoss = 0.0;
    double stepTX0 = 0.0;
  };

  struct LinkRecord {
    int trackID = -1;
    int firstStepNumber = -1;
    int lastStepNumber = -1;
    int hookStepNumber = -1;
    int hookKind = 0;
    double hookFraction = -1.0;
    int provenanceType = 0;
    int status = 0;
  };

  static ObjectKey objectKey(const podio::ObjectID& id);
  static double momentumMagnitude(const edm4hep::Vector3f& momentum);

  std::map<StepKey, StepRecord> m_steps;
  std::map<ObjectKey, LinkRecord> m_linksBySimHit;
  std::map<ObjectKey, std::set<ObjectKey>> m_simHitsByRecoHit;
};

#endif
