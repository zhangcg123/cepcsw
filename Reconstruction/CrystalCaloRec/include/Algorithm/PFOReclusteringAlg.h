#ifndef _PFORECLUSTERING_ALG_H
#define _PFORECLUSTERING_ALG_H

#include "Tools/Algorithm.h"

using namespace PandoraPlus;
class PFOReclusteringAlg: public PandoraPlus::Algorithm{
public: 

  PFOReclusteringAlg(){};
  ~PFOReclusteringAlg(){};

  class Factory : public PandoraPlus::AlgorithmFactory
  {
  public: 
    PandoraPlus::Algorithm* CreateAlgorithm() const{ return new PFOReclusteringAlg(); } 

  };

  StatusCode ReadSettings(PandoraPlus::Settings& m_settings);
  StatusCode Initialize( PandoraPlusDataCol& m_datacol );
  StatusCode RunAlgorithm( PandoraPlusDataCol& m_datacol );
  StatusCode ClearAlgorithm();

  StatusCode ReCluster_MergeToChg( std::vector< std::shared_ptr<PandoraPlus::PFObject> >& m_chargedPFOs,
                                   std::vector< std::shared_ptr<PandoraPlus::PFObject> >& m_neutralPFOs );

  StatusCode ReCluster_SplitFromChg( std::vector< std::shared_ptr<PandoraPlus::PFObject> >& m_chargedPFOs,
                                     std::vector< std::shared_ptr<PandoraPlus::PFObject> >& m_neutralPFOs );

private: 
  
  std::vector<std::shared_ptr<PandoraPlus::PFObject>>* p_PFObjects; 

  PandoraPlusDataCol m_bkCol;

  static bool compTrkP( std::shared_ptr<PandoraPlus::PFObject> pfo1, std::shared_ptr<PandoraPlus::PFObject> pfo2 )
    { return pfo1->getTrackMomentum() > pfo2->getTrackMomentum(); }
};
#endif
