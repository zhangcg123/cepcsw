#include "MarlinArbor.hh"
#include "ArborTool.h"
#include "ArborToolLCIO.hh"
#include "ArborHit.h"

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "Gaudi/Property.h"
#include "edm4hep/EventHeader.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/Cluster.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCParticleCollection.h"

#include "cellIDDecoder.h"
#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include "DetInterface/IGeomSvc.h"

#include "DecoderHelper/DD4hep2Lcio.h"

#include "DD4hep/Detector.h"
#include "DD4hep/IDDescriptor.h"
#include "DD4hep/Plugins.h"

#include <values.h>
#include <string>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <Rtypes.h>
#include <sstream>
#include <set>
#include <TVector3.h>
#include <vector>
#include <algorithm>

#include "DetectorPos.hh"

using namespace std;

extern linkcoll InitLinks;
extern linkcoll IterLinks_1;
extern linkcoll IterLinks;
extern linkcoll links_debug;
extern branchcoll Trees;
extern std::vector<int> IsoHitsIndex;

//std::vector<std::string> CaloHitCollections;

DECLARE_COMPONENT(MarlinArbor)

MarlinArbor::MarlinArbor(const std::string& name, ISvcLocator* svcLoc)
     : GaudiAlgorithm(name, svcLoc),
          _eventNr(0),_output(0)
{
}

StatusCode MarlinArbor::initialize() {

// ???
	// _cepc_thresholds.push_back(10);
	// _cepc_thresholds.push_back(90);
	// _cepc_thresholds.push_back(50);
	// _cepc_thresholds.push_back(7.5);

	m_encoder_str = "M:3,S-1:3,I:9,J:9,K-1:6";
    _cepc_thresholds.push_back(20);
	_cepc_thresholds.push_back(90);
	_cepc_thresholds.push_back(50);
	_cepc_thresholds.push_back(11);

     m_geosvc = service<IGeomSvc>("GeomSvc");

      ISvcLocator* svcloc = serviceLocator();
    //   m_ArborToolLCIO=new ArborToolLCIO("arborTools",svcloc);
      m_ArborToolLCIO=new ArborToolLCIO("arborTools",svcloc,m_readLCIO);
         for(unsigned int i = 0; i < m_ecalReadoutNames.value().size(); i++){
             m_col_readout_map[m_ecalColNames.value().at(i)] = m_ecalReadoutNames.value().at(i);
         }
         for(unsigned int i = 0; i < m_hcalReadoutNames.value().size(); i++){
             m_col_readout_map[m_hcalColNames.value().at(i)] = m_hcalReadoutNames.value().at(i);
         }

     for (auto& ecal : m_ecalColNames) {
	  _ecalCollections.push_back( new CaloType(ecal, Gaudi::DataHandle::Reader, this) );
	  _calCollections.push_back( new CaloType(ecal, Gaudi::DataHandle::Reader, this) );
     }
     for (auto& hcal : m_hcalColNames) {
	  _hcalCollections.push_back( new CaloType(hcal, Gaudi::DataHandle::Reader, this) );
	  _calCollections.push_back( new CaloType(hcal, Gaudi::DataHandle::Reader, this) );
     }
	return GaudiAlgorithm::initialize();
}

void MarlinArbor::HitsPreparation()
{
	cout<<"Start to prepare Hits"<<endl;
}

void MarlinArbor::MakeIsoHits( std::vector<edm4hep::CalorimeterHit> inputCaloHits, DataHandle<edm4hep::CalorimeterHitCollection>& m_hitcol)
{
	edm4hep::CalorimeterHitCollection* isohitcoll = m_hitcol.createAndPut();

	int nhit = inputCaloHits.size();

	for(int i = 0; i < nhit; i++)
	{
		auto a_hit = inputCaloHits[i];
		auto IsoHit = isohitcoll->create();
		IsoHit.setPosition(a_hit.getPosition());
		IsoHit.setCellID(a_hit.getCellID());
		IsoHit.setEnergy(a_hit.getEnergy());
		//isohitcoll->addElement(collhit);
	}

}

StatusCode MarlinArbor::execute()
{
     //if(_eventNr % m_reportEvery == 0) cout<<"eventNr: "<<_eventNr<<endl;
     _eventNr++;

	MarlinArbor::HitsPreparation();	//Absorb isolated hits;

	TVector3 currHitPos;

	std::vector< TVector3 > inputHitsPos;
	std::vector< ArborHit > inputABHit;
	std::vector< edm4hep::CalorimeterHit > inputHits;
	std::vector< edm4hep::CalorimeterHit > inputECALHits;
	std::vector< edm4hep::CalorimeterHit > inputHCALHits;
	std::vector< std::vector<int> > Sequence;
	int LayerNum = 0;
	int StaveNum = 0;
	int SubDId = -10;
	float Depth = 0;
	int KShift = 0;
	TVector3 TrkEndPointPos;
	std::vector<edm4hep::CalorimeterHit> IsoHits;


    cout<<"[YX debug - MarlinArbor] InputCollections.size() = "<<_calCollections.size()<<endl;

    unsigned int nECALCol = m_ecalColNames.size();
    unsigned int nHCALCol = m_hcalColNames.size();

	for(unsigned int i1 = 0; i1 < _calCollections.size(); i1++)
	{

		// cout<<"[YX debug - MarlinArbor] CaloHitCollections "<<i1<<endl;

		if(i1<nECALCol)
            std::cout<<"[YX debug - MarlinArbor] CaloHitCollections "<<i1<<": "<<m_col_readout_map[m_ecalColNames.value().at(i1)]<<std::endl;
        else
            std::cout<<"[YX debug - MarlinArbor] CaloHitCollections "<<i1<<": "<<m_col_readout_map[m_hcalColNames.value().at(i1-nECALCol)]<<std::endl;


		std::string tmp_readout;

	      //   if(i1<2)tmp_readout = m_col_readout_map[m_ecalColNames.value().at(i1)];
        if(i1<nECALCol)
            tmp_readout = m_col_readout_map[m_ecalColNames.value().at(i1)];
        else
            tmp_readout = m_col_readout_map[m_hcalColNames.value().at(i1-nECALCol)];
        //   tmp_readout = m_col_readout_map[m_hcalColNames.value().at(i1-2)];

	      std::cout<<"tmp_readout: "<<tmp_readout<<std::endl;

            // // get the DD4hep readout
              // m_decoder = m_geosvc->getDecoder(tmp_readout);

			KShift = 0;
			SubDId = -1;

			// ???
			// if( i1 < _EcalCalCollections.size() )
            // 	SubDId = 1;
			// else if( i1 < _EcalCalCollections.size() + _HcalCalCollections.size() )
			// 	SubDId = 2;
			// else
			// 	SubDId = 3;

			// if(i1 >  _EcalCalCollections.size() - 1)
			// 	KShift = 100;
			// else if( i1 == _calCollections.size() - 2)	//HCAL Ring
			// 	KShift = 50;

			if( i1 < _ecalCollections.size() )
				SubDId = 1;
			else if( i1 < _ecalCollections.size() + _hcalCollections.size() )
				SubDId = 2;
			else
				SubDId = 3;

			if(i1 >  _ecalCollections.size() - 1)
				KShift = 100;
			else if( i1 == _calCollections.size() - 2)	//HCAL Ring
				KShift = 50;
            // !!!

			auto CaloHitColl = _calCollections[i1]->get();

            int i2 = 0;

			//int NHitsCurrCol = CaloHitColl->getNumberOfElements();
			//CellIDDecoder<CalorimeterHit> idDecoder(CaloHitColl);
			for(auto a_hit: *CaloHitColl){
                ID_UTIL::CellIDDecoder<edm4hep::CalorimeterHit> cellIdDecoder(m_encoder_str);
                const std::string layerCodingString(m_encoder_str);
                const std::string staveCodingString(m_encoder_str);
	      		const std::string idCodingString(m_encoder_str);

                const std::string staveCoding(m_ArborToolLCIO->GetStaveCoding(staveCodingString));
                const std::string layerCoding(m_ArborToolLCIO->GetLayerCoding(layerCodingString));
                // const std::string layerCoding(m_ArborToolLCIO->GetLayerCoding(idCodingString));
	      		const std::string cellICoding(m_ArborToolLCIO->GetCellICoding(idCodingString));
	      		const std::string cellJCoding(m_ArborToolLCIO->GetCellJCoding(idCodingString));

                if(!m_readLCIO)
                    m_decoder = m_geosvc->getDecoder(tmp_readout);

                // m_decoder = m_geosvc->getDecoder("EcalBarrelCollection");
                // if(!m_decoder) m_decoder = m_geosvc->getDecoder("EcalEndcapsCollection");

                currHitPos =  TVector3(a_hit.getPosition().x, a_hit.getPosition().y, a_hit.getPosition().z);
                Depth = DisSeedSurface(currHitPos);

                if(m_readLCIO){
                    LayerNum=cellIdDecoder(&a_hit)[layerCoding.c_str()] + 1 + KShift;
                    StaveNum=cellIdDecoder(&a_hit)[staveCoding.c_str()] + 1 ;

                    if(1){
                        double hitposx = currHitPos.X();
                        double hitposy = currHitPos.Y();
                        double hitposz = currHitPos.Z();
                        double hitposp = currHitPos.Perp();

                        // int tmp_M = idDecoder(a_hit)["M"];
                        int tmp_S = cellIdDecoder(&a_hit)[staveCoding.c_str()];
                        int tmp_K = cellIdDecoder(&a_hit)[layerCoding.c_str()];
                        int tmp_I = cellIdDecoder(&a_hit)[cellICoding.c_str()];
                        int tmp_J = cellIdDecoder(&a_hit)[cellJCoding.c_str()];

                        if(0) cout<<"[MarlinArbor] Hit Pos ("<<hitposx<<", "<<hitposy<<", "<<hitposz<<", "<<hitposp<<")mm:"<<endl;
                        if(0) cout<<"[MarlinArbor] ---> M = xx, stave = "<<tmp_S<<", layer = "<<tmp_K<<", I(x) = "<<tmp_I<<", J(y) = "<<tmp_J<<endl;
                        if(0) cout<<"[MarlinArbor] ---> StaveNum = "<<StaveNum<<", LayerNum = "<<LayerNum<<endl;
                    }

                }else{
				    auto cellid = a_hit.getCellID();


                    // SEcal05_siw_Barrel       <id>system:5,module:3,stave:4,tower:5,layer:6,wafer:6,cellX:32:-16,cellY:-16</id>
                    // SEcal05_siw_Endcaps      <id>system:5,module:3,stave:4,tower:5,layer:6,wafer:6,x:32:-16,y:-16</id>
                    // SEcal05_siw_ECRing_01    <id>system:5,module:3,stave:4,tower:3,layer:6,x:32:-16,y:-16</id>
                    // SHcalRpc01_Barrel_01     <id>system:5,module:3,stave:3,tower:5,layer:6,slice:4,x:32:-16,y:-16</id>
                    // SHcalRpc01_Endcaps_01    <id>system:5,module:3,stave:3,tower:5,layer:6,y:32:-16,x:-16</id>
                    // SHcalRpc01_EndcapRing_01 <id>system:5,module:3,stave:4,tower:3,layer:6,y:32:-16,x:-16</id>

                    int Raw_system = m_decoder->get(cellid, "system");
                    int Raw_module = m_decoder->get(cellid, "module");
                    int Raw_Stave = m_decoder->get(cellid, "stave");
                    int Raw_Layer = m_decoder->get(cellid, "layer");

                    int New_Layer = DD4hep2Lcio::CEPCv4::getEcalLayer(Raw_Layer);
                    int New_Stave = DD4hep2Lcio::CEPCv4::getEcalBarrelStave(Raw_Stave);
                    if(Raw_system==29){// ECAL endcap
                        New_Stave = DD4hep2Lcio::CEPCv4::getEcalEndcapStave(Raw_Stave);
                    }
                    if(Raw_system==22 || Raw_system==30){
                        New_Layer = DD4hep2Lcio::CEPCv4::getHcalLayer(Raw_Layer);
                        New_Stave = DD4hep2Lcio::CEPCv4::getHcalStave(Raw_Stave);
                    }
                    LayerNum = New_Layer + KShift + 1;
                    StaveNum = New_Stave + 1;

                    // LayerNum = m_decoder->get(cellid, "layer") + KShift;
                    // StaveNum = m_decoder->get(cellid, "stave");


                    if(0){
                        double hitposx = currHitPos.X();
                        double hitposy = currHitPos.Y();
                        double hitposz = currHitPos.Z();
                        double hitposp = currHitPos.Perp();

                        cout<<"[MarlinArbor] Hit Pos ("<<hitposx<<", "<<hitposy<<", "<<hitposz<<", "<<hitposp<<")mm:"<<endl;
                        // cout<<"---> stave = "<<Raw_Stave<<", layer = "<<Raw_Layer<<", X = "<<Raw_cellX<<", Y = "<<Raw_cellY<<", wafer = "<<Raw_Wafer<<", tower = "<<Raw_Tower<<endl;

                        cout<<"[MarlinArbor] ---> Raw M = "<<Raw_module<<", stave = "<<Raw_Stave<<", layer = "<<Raw_Layer<<", system = "<<Raw_system<<endl;
                        cout<<"[MarlinArbor] ---> New M = "<<Raw_module<<", stave = "<<New_Stave<<", layer = "<<New_Layer<<", system = "<<Raw_system<<endl;
                        cout<<"[MarlinArbor] ---> StaveNum = "<<StaveNum<<", LayerNum = "<<LayerNum<<endl;
                    }

                }

		       		if(SubDId!=2 ){
		       			inputECALHits.push_back(a_hit);
		       		}
		       		else{
		       			inputHCALHits.push_back(a_hit);
		       		}
		       		ArborHit a_abhit(currHitPos, LayerNum, 0, Depth, StaveNum, SubDId);
		       		inputABHit.push_back(a_abhit);
		       		inputHits.push_back(a_hit);


                    // cout<<"[YX debug - MarlinArbor] Hit "<<i2<<", KShift = "<<KShift<<", LayerNum = "<<LayerNum<<", StaveNum = "<<StaveNum<<", SubDId = "<<SubDId<<", Depth = "<<Depth<<endl;
                    // // auto cellID = a_hit.getCellID();
                    // // cout<<"[YX debug - MarlinArbor] ---> raw layerNum = "<<m_decoder->get(cellID, "layer")<<", staveNum = "<<m_decoder->get(cellID, "stave")<<endl;
                    // // cout<<"[YX debug - MarlinArbor] ---> raw cellID (DigiHit) = "<<cellID<<endl;




                    i2+=1;
		       }


			// cout<<i1<<"  Stat  "<<SubDId<<" ~~~ "<<inputABHit.size()<<endl;

	}
	//cout<<"hit size"<<inputHits.size()<<endl;

	Sequence = Arbor(inputABHit, _cepc_thresholds);

    cout<<"[YX debug - MarlinArbor] inputABHit.size() = "<<inputABHit.size()<<endl;
    cout<<"[YX debug - MarlinArbor] Sequence.size() = "<<Sequence.size()<<endl;

	m_ArborToolLCIO->ClusterBuilding( branchCol, inputHits, Trees, 0 );

	for(unsigned int i2 = 0; i2 < IsoHitsIndex.size(); i2++)
	{
		auto a_Isohit = inputHits[ IsoHitsIndex[i2] ];
		if(a_Isohit.getEnergy() > 0)	//Veto Trk End Hits
		{
			IsoHits.push_back(a_Isohit);
		}
	}

	MakeIsoHits(IsoHits, m_isohitcol);

    cout<<"[YX debug - MarlinArbor] End."<<endl;

	return StatusCode::SUCCESS;
}


StatusCode MarlinArbor::finalize()
{
	std::cout<<"Arbor Ends. Good luck"<<std::endl;
	return GaudiAlgorithm::finalize();
}
