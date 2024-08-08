#ifndef _TRACKCLUSTERCONNECTING_ALG_H
#define _TRACKCLUSTERCONNECTING_ALG_H

#include "Tools/Algorithm.h"

using namespace PandoraPlus;
class TrackClusterConnectingAlg: public PandoraPlus::Algorithm{
public: 

  TrackClusterConnectingAlg(){};
  ~TrackClusterConnectingAlg(){};

  class Factory : public PandoraPlus::AlgorithmFactory
  {
  public: 
    PandoraPlus::Algorithm* CreateAlgorithm() const{ return new TrackClusterConnectingAlg(); } 

  };

  StatusCode ReadSettings(PandoraPlus::Settings& m_settings);
  StatusCode Initialize( PandoraPlusDataCol& m_datacol );
  StatusCode RunAlgorithm( PandoraPlusDataCol& m_datacol );
  StatusCode ClearAlgorithm();

  //Self defined algorithms
  double GetMinR2Trk( const PandoraPlus::Calo3DCluster* p_clus, const PandoraPlus::Track* m_trk);
  StatusCode PFOCreating( std::vector<const PandoraPlus::Calo3DCluster*>& m_clusters,
                          std::vector<const PandoraPlus::Track*>& m_trks,
                          std::vector<std::shared_ptr<PandoraPlus::PFObject>>& m_PFOs );

  StatusCode EcalChFragAbsorption( std::vector<const PandoraPlus::Calo3DCluster*>& m_clusters,
                                   std::vector<const PandoraPlus::Track*>& m_trks,
                                   std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>>& m_newclusCol ); 

  StatusCode HcalExtrapolatingMatch(std::vector<const PandoraPlus::Calo3DCluster*>& m_clusters, std::vector<std::shared_ptr<PandoraPlus::PFObject>>& m_PFOs);

private: 

  std::vector<const PandoraPlus::Calo3DCluster*> m_EcalClusters;
  std::vector<const PandoraPlus::Calo3DCluster*> m_HcalClusters;
  std::vector<const PandoraPlus::Track*> m_tracks; 

  std::vector<std::shared_ptr<PandoraPlus::Calo3DCluster>> m_absorbedEcal; 
  std::vector<std::shared_ptr<PandoraPlus::PFObject>> m_PFObjects; 

  PandoraPlusDataCol m_bkCol;

  static bool compTrkP( std::shared_ptr<PandoraPlus::PFObject> pfo1, std::shared_ptr<PandoraPlus::PFObject> pfo2 )
    { return pfo1->getTrackMomentum() > pfo2->getTrackMomentum(); }

};
#endif
