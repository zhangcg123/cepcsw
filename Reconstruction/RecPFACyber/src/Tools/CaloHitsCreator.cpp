#ifndef TRACK_CREATOR_C
#define TRACK_CREATOR_C

#include "Tools/CaloHitsCreator.h"

namespace Cyber{
  CaloHitsCreator::CaloHitsCreator(const Settings& m_settings) : settings( m_settings ){

  }; 

  StatusCode CaloHitsCreator::CreateCaloHits( CyberDataCol& m_DataCol, 
                                              std::vector<DataHandle<edm4hep::CalorimeterHitCollection>*>& r_CaloHitCols, 
                                              std::map<std::string, dd4hep::DDSegmentation::BitFieldCoder*>& map_decoder,
                                              std::map<std::string, DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>*>& map_CaloParticleAssoCol,
                                              SmartIF<IGeomSvc>& m_geosvc,
                                              std::map<std::tuple<int, int, int, int, int>, int>& barNumberMapEndcapMap )
  {
    if(r_CaloHitCols.size()==0 || settings.map_stringVecPars.at("CaloHitCollections").size()==0) StatusCode::SUCCESS;

    //Save readin collections
    m_DataCol.collectionMap_CaloHit.clear(); 
    for(unsigned int icol=0; icol<r_CaloHitCols.size(); icol++){

      const edm4hep::CalorimeterHitCollection* const_CaloHitCol = r_CaloHitCols[icol]->get(); 

      std::vector<edm4hep::CalorimeterHit> m_HitCol; m_HitCol.clear(); 
      for(unsigned int ihit=0; ihit<const_CaloHitCol->size(); ihit++){
        edm4hep::CalorimeterHit m_hit = const_CaloHitCol->at(ihit);
        m_HitCol.push_back(m_hit);
      }

      m_DataCol.collectionMap_CaloHit[settings.map_stringVecPars.at("CaloHitCollections")[icol]] = m_HitCol; 
    }

    //Convert to local objects: 
    for(auto iter : m_DataCol.collectionMap_CaloHit){
      if( settings.map_stringPars.at("EcalType")=="BarEcal" && (iter.first == "ECALBarrel" || iter.first == "ECALEndcaps" )) continue; 
      
      std::vector<std::shared_ptr<Cyber::CaloHit>> m_hitCol; m_hitCol.clear();
      const edm4hep::MCRecoCaloParticleAssociationCollection* const_MCPCaloAssoCol;
      if( map_CaloParticleAssoCol.find(iter.first)!=map_CaloParticleAssoCol.end()) 
        const_MCPCaloAssoCol = map_CaloParticleAssoCol[iter.first]->get();

      for(int ihit=0; ihit<iter.second.size(); ihit++){
        //Cyber::CaloHit* m_hit = new Cyber::CaloHit();
        std::shared_ptr<Cyber::CaloHit> m_hit = std::make_shared<Cyber::CaloHit>();

        m_hit->setOriginHit( iter.second[ihit] );
        m_hit->setcellID( iter.second[ihit].getCellID() );
        m_hit->setLayer( map_decoder[iter.first]->get(iter.second[ihit].getCellID(), "layer") );

        TVector3 pos( iter.second[ihit].getPosition().x, iter.second[ihit].getPosition().y, iter.second[ihit].getPosition().z );
        m_hit->setPosition( pos );
        m_hit->setEnergy( iter.second[ihit].getEnergy() );

        for(int ilink=0; ilink<const_MCPCaloAssoCol->size(); ilink++){
          if( iter.second[ihit] == const_MCPCaloAssoCol->at(ilink).getRec() ) m_hit->addLinkedMCP( std::make_pair(const_MCPCaloAssoCol->at(ilink).getSim(), const_MCPCaloAssoCol->at(ilink).getWeight()) );
        }
        m_hitCol.push_back( m_hit );
      }
      m_DataCol.map_CaloHit[iter.first] = m_hitCol;
      const_MCPCaloAssoCol = nullptr;
    }


    //Convert to local objects: CalorimeterHit to CaloUnit (For ECALBarrel only)
    if(settings.map_stringPars.at("EcalType")=="BarEcal"){
      std::vector<std::shared_ptr<Cyber::CaloUnit>> m_barCol; m_barCol.clear(); 

      //Readin ECAL barrel hits      
      if( m_DataCol.collectionMap_CaloHit.find("ECALBarrel") != m_DataCol.collectionMap_CaloHit.end() ){
        const edm4hep::MCRecoCaloParticleAssociationCollection* const_MCPCaloAssoCol = map_CaloParticleAssoCol["ECALBarrel"]->get();   
        auto CaloHits = m_DataCol.collectionMap_CaloHit["ECALBarrel"]; 
        std::map<std::uint64_t, std::vector<Cyber::CaloUnit> > map_cellID_hits; map_cellID_hits.clear();
        for(auto& hit : CaloHits){ 
          Cyber::CaloUnit m_bar; 
          m_bar.setcellID(hit.getCellID());
          m_bar.setPosition( TVector3(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z) );
          m_bar.setQ(hit.getEnergy()/2., hit.getEnergy()/2.);
          m_bar.setT(hit.getTime(), hit.getTime());

          //unsigned long long tmp_id = hit.getCellID();
          //dd4hep::PlacedVolume ipv = m_volumeManager.lookupVolumePlacement(tmp_id);
          //dd4hep::Volume ivol = ipv.volume();
          //std::vector< double > iVolParam = ivol.solid().dimensions();
          //auto maxElement = std::max_element(iVolParam.begin(), iVolParam.end());
          //iVolParam.clear();
          //m_bar.setBarLength(*maxElement * 20);
          m_bar.setBarLength(m_geosvc->getEcalBarLength(hit.getCellID()));

          for(int ilink=0; ilink<const_MCPCaloAssoCol->size(); ilink++){
            if( hit == const_MCPCaloAssoCol->at(ilink).getRec() ) m_bar.addLinkedMCP( std::make_pair(const_MCPCaloAssoCol->at(ilink).getSim(), const_MCPCaloAssoCol->at(ilink).getWeight()) );
          }
 
          map_cellID_hits[hit.getCellID()].push_back(m_bar);
        }
        for(auto& hit : map_cellID_hits){
          if(hit.second.size()!=2){ std::cout<<"WARNING: didn't find correct hit pairs! "<<std::endl; continue; }
   
          //Cyber::CaloUnit* m_bar = new Cyber::CaloUnit(); 
          std::shared_ptr<Cyber::CaloUnit> m_bar = std::make_shared<Cyber::CaloUnit>();
   
          unsigned long long id = hit.first; 
          m_bar->setcellID( id );
          m_bar->setcellID( map_decoder["ECALBarrel"]->get(id, "system"),
                            map_decoder["ECALBarrel"]->get(id, "module"),
                            map_decoder["ECALBarrel"]->get(id, "stave"),
                            -1,                                           //empty 'part' for barrel. 
                            map_decoder["ECALBarrel"]->get(id, "dlayer"),
                            map_decoder["ECALBarrel"]->get(id, "slayer"),
                            map_decoder["ECALBarrel"]->get(id, "bar"));
          m_bar->setPosition(hit.second[0].getPosition());
          m_bar->setBarLength(hit.second[0].getBarLength());  
          m_bar->setQ( hit.second[0].getEnergy(), hit.second[1].getEnergy() );
          m_bar->setT( hit.second[0].getT1(), hit.second[1].getT1() );

          //add MCParticle link
          for(int ilink=0; ilink<hit.second[0].getLinkedMCP().size(); ilink++) m_bar->addLinkedMCP( hit.second[0].getLinkedMCP()[ilink] );

          m_barCol.push_back(m_bar);  //Save for later use in algorithms
        }
   
        m_DataCol.map_BarCol["BarCol"] = m_barCol; 
        const_MCPCaloAssoCol = nullptr;
      }

      m_barCol.clear();
      if( m_DataCol.collectionMap_CaloHit.find("ECALEndcaps") != m_DataCol.collectionMap_CaloHit.end() ){
        const edm4hep::MCRecoCaloParticleAssociationCollection* const_MCPCaloAssoCol = map_CaloParticleAssoCol["ECALEndcaps"]->get();
        auto CaloHits = m_DataCol.collectionMap_CaloHit["ECALEndcaps"];
        std::map<std::uint64_t, std::vector<Cyber::CaloUnit> > map_cellID_hits; map_cellID_hits.clear();
        for(auto& hit : CaloHits){
          Cyber::CaloUnit m_bar;
          m_bar.setcellID(hit.getCellID());
          m_bar.setPosition( TVector3(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z) );
          m_bar.setQ(hit.getEnergy()/2., hit.getEnergy()/2.);
          m_bar.setT(hit.getTime(), hit.getTime());

          //unsigned long long tmp_id = hit.getCellID();
          //dd4hep::PlacedVolume ipv = m_volumeManager.lookupVolumePlacement(tmp_id);
          //dd4hep::Volume ivol = ipv.volume();
          //std::vector< double > iVolParam = ivol.solid().dimensions();
          //auto maxElement = std::max_element(iVolParam.begin(), iVolParam.end());
          //iVolParam.clear();
          //m_bar.setBarLength(*maxElement * 20);
          m_bar.setBarLength(m_geosvc->getEcalBarLength(hit.getCellID()));

          for(int ilink=0; ilink<const_MCPCaloAssoCol->size(); ilink++){
            if( hit == const_MCPCaloAssoCol->at(ilink).getRec() ) m_bar.addLinkedMCP( std::make_pair(const_MCPCaloAssoCol->at(ilink).getSim(), const_MCPCaloAssoCol->at(ilink).getWeight()) );
          }


          map_cellID_hits[hit.getCellID()].push_back(m_bar);
        }
        for(auto& hit : map_cellID_hits){
          if(hit.second.size()!=2){ std::cout<<"WARNING: didn't find correct hit pairs! "<<std::endl; continue; }

          //Cyber::CaloUnit* m_bar = new Cyber::CaloUnit();
          std::shared_ptr<Cyber::CaloUnit> m_bar = std::make_shared<Cyber::CaloUnit>();

          unsigned long long id = hit.first;
          m_bar->setcellID( id );
          m_bar->setcellID( map_decoder["ECALEndcaps"]->get(id, "system"),
                            map_decoder["ECALEndcaps"]->get(id, "module"),
                            map_decoder["ECALEndcaps"]->get(id, "stave"),
                            map_decoder["ECALEndcaps"]->get(id, "part"),
                            map_decoder["ECALEndcaps"]->get(id, "dlayer"),
                            map_decoder["ECALEndcaps"]->get(id, "slayer"),
                            map_decoder["ECALEndcaps"]->get(id, "bar"));
          m_bar->setNBarInLayer(barNumberMapEndcapMap[std::make_tuple(m_bar->getModule(), m_bar->getStave(), m_bar->getPart(), m_bar->getDlayer(), m_bar->getSlayer())]);
          m_bar->setPosition(hit.second[0].getPosition());
          m_bar->setBarLength(hit.second[0].getBarLength());
          m_bar->setQ( hit.second[0].getEnergy(), hit.second[1].getEnergy() );
          m_bar->setT( hit.second[0].getT1(), hit.second[1].getT1() );

          //add MCParticle link
          for(int ilink=0; ilink<hit.second[0].getLinkedMCP().size(); ilink++) m_bar->addLinkedMCP( hit.second[0].getLinkedMCP()[ilink] );

          m_barCol.push_back(m_bar);  //Save for later use in algorithms
        }

        m_DataCol.map_BarCol["BarCol"].insert( m_DataCol.map_BarCol["BarCol"].end(), m_barCol.begin(), m_barCol.end() );
        const_MCPCaloAssoCol = nullptr;      
      }

    }
    return StatusCode::SUCCESS;
  };

};

#endif
