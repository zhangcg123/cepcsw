#ifndef TPCDndxAlg_h
#define TPCDndxAlg_h 1

#include "k4FWCore/DataHandle.h"

#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/RecDqdx.h"
#include "edm4hep/RecDqdxCollection.h"

#include "GaudiAlg/GaudiAlgorithm.h"

#include "DetInterface/IGeomSvc.h"
#include "SimplePIDSvc/ISimplePIDSvc.h"

/**
 * @class TPCDndxAlg
 * @brief Algorithm to calculate dN/dx for TPC
 * @author Guang Zhao (zhaog@ihep.ac.cn)
*/

class TPCDndxAlg : public GaudiAlgorithm {
public:
    TPCDndxAlg(const std::string& name, ISvcLocator* svcLoc);

    virtual StatusCode initialize();
    virtual StatusCode execute();
    virtual StatusCode finalize();

protected:
    Gaudi::Property<std::string> m_method{this, "Method", "Simple"};
    SmartIF<IGeomSvc> m_geosvc;
    dd4hep::DDSegmentation::BitFieldCoder* m_decoder;

    DataHandle<edm4hep::TrackCollection> _trackCol{"CompleteTracks", Gaudi::DataHandle::Reader, this};
    DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> _trkParAssCol{"CompleteTracksParticleAssociation", Gaudi::DataHandle::Reader, this};
    DataHandle<edm4hep::RecDqdxCollection> _dndxCol{"DndxTracks", Gaudi::DataHandle::Writer, this};

private:
    SmartIF<ISimplePIDSvc> m_pid_svc;
    void getFirstAndLastHitsByRadius(const podio::RelationRange<edm4hep::TrackerHit>& hitcol, int& first, int& last);
    void getFirstAndLastHitsByZ(const podio::RelationRange<edm4hep::TrackerHit>& hitcol, int& first, int& last);
};

#endif
