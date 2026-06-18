#ifndef ARBORTOOLLCIO_H_
#define ARBORTOOLLCIO_H_


#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/EventHeaderCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackMCParticleLinkCollection.h"
#include "edm4hep/CaloHitMCParticleLinkCollection.h"
#include "edm4hep/CaloHitSimCaloHitLinkCollection.h"
#else
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoCaloAssociation.h"
#include "edm4hep/MCRecoTrackerAssociation.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#endif
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/VertexCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticle.h"
#include "edm4hep/MCParticleCollection.h"

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
using CEPCSWCaloHitMCParticleLink = edm4hep::CaloHitMCParticleLink;
using CEPCSWCaloHitMCParticleLinkCollection = edm4hep::CaloHitMCParticleLinkCollection;
using CEPCSWTrackMCParticleLink = edm4hep::TrackMCParticleLink;
using CEPCSWTrackMCParticleLinkCollection = edm4hep::TrackMCParticleLinkCollection;
using CEPCSWCaloHitSimCaloHitLink = edm4hep::CaloHitSimCaloHitLink;
using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::CaloHitSimCaloHitLinkCollection;
#else
using CEPCSWCaloHitMCParticleLink = edm4hep::MCRecoCaloAssociation;
using CEPCSWCaloHitMCParticleLinkCollection = edm4hep::MCRecoCaloAssociationCollection;
using CEPCSWTrackMCParticleLink = edm4hep::MCRecoTrackerAssociation;
using CEPCSWTrackMCParticleLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
using CEPCSWCaloHitSimCaloHitLink = edm4hep::MCRecoCaloAssociation;
using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::MCRecoCaloAssociationCollection;
#endif


#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include "DetInterface/IGeomSvc.h"

#include "TVector3.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/MsgStream.h"
#include "ArborTool.h"
#include "HelixClassD.hh"

class CollectionMaps
{
public:
    CollectionMaps();
    void clear();
    std::map<std::string, std::vector<edm4hep::MCParticle> >     collectionMap_MC;
    std::map<std::string, std::vector<edm4hep::CalorimeterHit> > collectionMap_CaloHit;
    std::map<std::string, std::vector<edm4hep::Vertex> >         collectionMap_Vertex;
    std::map<std::string, std::vector<edm4hep::Track> >          collectionMap_Track;
    std::map<std::string, std::vector<CEPCSWCaloHitSimCaloHitLink> > collectionMap_CaloRel;
    std::map<std::string, std::vector<CEPCSWTrackMCParticleLink> > collectionMap_TrkRel;
};



class ArborToolLCIO  : public Algorithm
{
	public:
		ArborToolLCIO(const std::string& name, ISvcLocator* svcLoc,  bool m_readLCIO);
		//ArborToolLCIO(ISvcLocator* svcLoc);
		~ArborToolLCIO();
		StatusCode execute() override { return StatusCode::SUCCESS; }
		typedef DataHandle<edm4hep::Cluster>  ClusterType;

        // bool _USE_LCIO;


    void Clean();
	void ClusterBuilding(DataHandle<edm4hep::ClusterCollection>& _currbranchcoll, std::vector<edm4hep::CalorimeterHit> Hits, branchcoll BranchOrder, int DHCALFlag);

	float ClusterT0(edm4hep::Cluster a_Clu);

	int NHScaleV2( std::vector<edm4hep::CalorimeterHit> clu0, int RatioX, int RatioY, int RatioZ );
	float FDV2( std::vector<edm4hep::CalorimeterHit> clu);

	int NHScaleV3( edm4hep::Cluster clu0, int RatioX, int RatioY, int RatioZ );

	float FDV3( edm4hep::Cluster clu);

	int ActiveLayers(  std::vector<edm4hep::CalorimeterHit> clu );

	float BushDis( edm4hep::Cluster clu1, edm4hep::Cluster clu2);


	float* SimpleDisTrackClu(edm4hep::Track a_trk, edm4hep::Cluster a_clu);

	float SimpleBushTimeTrackClu(edm4hep::Track a_trk, edm4hep::Cluster a_clu);

	int SimpleBushNC(edm4hep::Track a_trk, edm4hep::Cluster a_clu);

	float DisPointToBush( TVector3 Pos1, edm4hep::Cluster clu1);

	TVector3 ClusterCoG(edm4hep::Cluster inputCluser);

	edm4hep::ClusterCollection* ClusterVecMerge( std::vector<edm4hep::Cluster> inputClusters, TMatrixF ConnectorMatrix, DataHandle<edm4hep::ClusterCollection>& clucol );

	edm4hep::ClusterCollection* ClusterVecColl( std::vector<edm4hep::MutableCluster> inputClusters, DataHandle<edm4hep::ClusterCollection>& m_clucol);

	edm4hep::Cluster NaiveCluImpl(edm4hep::MutableCluster a0_clu);
	void NaiveCluConst(edm4hep::MutableCluster a0_clu, edm4hep::MutableCluster b0_clu);

	std::vector<edm4hep::Cluster> CollClusterVec(const edm4hep::ClusterCollection * input_coll );

	std::vector<edm4hep::CalorimeterHit> CollHitVec(const edm4hep::CalorimeterHitCollection * input_coll, float EnergyThreshold);

	std::vector<edm4hep::MutableCluster> ClusterHitAbsorbtion( std::vector<edm4hep::Cluster> MainClusters, std::vector<edm4hep::CalorimeterHit> IsoHits, float DisThreshold );

	edm4hep::MutableCluster NaiveMergeClu(std::vector<edm4hep::Cluster> inputCluVec);

	void NaiveMergeCluConst(std::vector<edm4hep::Cluster> inputCluVec,edm4hep::MutableCluster MergedClu);
	std::vector<edm4hep::MutableCluster> ClusterAbsorbtion( std::vector<edm4hep::Cluster> MainClusters, std::vector<edm4hep::MutableCluster> FragClusters, float thresholds, float DepthSlope );


	int JointsBetweenBush(edm4hep::Cluster a_Clu, edm4hep::Cluster b_Clu, float CellSize);

	TVector3 CluEndP(edm4hep::Cluster a_clu);

	int ClusterFlag(edm4hep::Cluster a_tree, edm4hep::Track a_trk);

	int ClusterFlag1st(edm4hep::Cluster a_tree);

	int newPhotonTag(edm4hep::Cluster a_clu);
	float ClusterEE(edm4hep::Cluster inputCluster);

	float EMClusterEE( edm4hep::Cluster inputCluster );

	std::vector<float> ClusterTime(edm4hep::Cluster inputCluster);

    std::string GetCellICoding(const std::string &encodingString) const;
    std::string GetCellJCoding(const std::string &encodingString) const;
    std::string GetStaveCoding(const std::string &encodingString) const;
    std::string GetLayerCoding(const std::string &encodingString) const;

	private:
	SmartIF<IGeomSvc> m_geosvc;//=service<IGeomSvc>("GeomSvc");
	dd4hep::DDSegmentation::BitFieldCoder*	m_decoder;// = m_geosvc->getDecoder("ECALBarrel");


    std::string m_encoder_str;
//	m_geosvc= service<IGeomSvc>("GeomSvc");
//	m_decoder = m_geosvc->getDecoder("ECALBarrel");
    bool _USE_LCIO;

};
#endif //
