#include "Edm4hepWriterAnaElemTool.h"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4THitsCollection.hh"
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4GammaGeneralProcess.hh"

#include "DD4hep/Detector.h"
#include "DD4hep/Plugins.h"
#include "DDG4/Geant4Converter.h"
#include "DDG4/Geant4Mapping.h"
#include "DDG4/Geant4HitCollection.h"
#include "DDG4/Geant4Data.h"
#include "DetSimSD/Geant4Hits.h"

DECLARE_COMPONENT(Edm4hepWriterAnaElemTool)

void
Edm4hepWriterAnaElemTool::BeginOfRunAction(const G4Run*) {
    auto msglevel = msgLevel();
    if (msglevel == MSG::VERBOSE || msglevel == MSG::DEBUG) {
        verboseOutput = true;
    }

    G4cout << "Begin Run of detector simultion..." << G4endl;

    // access geometry service
    m_geosvc = service<IGeomSvc>("GeomSvc");
    if (m_geosvc) {
        dd4hep::Detector* dd4hep_geo = m_geosvc->lcdd();
        try {
            // try to get the constants
            R = dd4hep_geo->constant<double>("tracker_region_rmax")/dd4hep::mm*CLHEP::mm;
            Z = dd4hep_geo->constant<double>("tracker_region_zmax")/dd4hep::mm*CLHEP::mm;
        } catch (std::runtime_error&e) {
            warning() << "constant tracker_region_rmax and tracker_region_zmax is not defined. " << endmsg;
        }

        info() << "Tracker Region "
               << " R: " << R
               << " Z: " << Z
               << endmsg;

    } else {
        error() << "Failed to find GeomSvc." << endmsg;
    }

}

void
Edm4hepWriterAnaElemTool::EndOfRunAction(const G4Run*) {
    G4cout << "End Run of detector simultion..." << G4endl;
}

void
Edm4hepWriterAnaElemTool::BeginOfEventAction(const G4Event* anEvent) {
    msg() << "Event " << anEvent->GetEventID() << endmsg;

    // event info
    m_userinfo = nullptr;
    if (anEvent->GetUserInformation()) {
        m_userinfo = dynamic_cast<CommonUserEventInfo*>(anEvent->GetUserInformation());
        if (verboseOutput) {
            m_userinfo->dumpIdxG4Track2Edm4hep();
        }
    }

    auto mcGenCol = m_mcParGenCol.get();
    mcCol = m_mcParCol.createAndPut();

    // ========================================================================
    // deep clone the MC particle
    // ========================================================================
    // In previous version, the mcGenParticle.getParents/getDaughters are used
    // to add the parents/daughters to the new particle. However, this is not
    // correct. Because the information is referred to the original collection.
    //
    // In order to fix this issue, we need to access the index.

    for (auto mcGenParticle: *mcGenCol) {
        auto newparticle = mcCol->create();
        newparticle.setPDG            (mcGenParticle.getPDG());
        newparticle.setGeneratorStatus(mcGenParticle.getGeneratorStatus());
        newparticle.setSimulatorStatus(mcGenParticle.getSimulatorStatus());
        newparticle.setCharge         (mcGenParticle.getCharge());
        newparticle.setTime           (mcGenParticle.getTime());
        newparticle.setMass           (mcGenParticle.getMass());
        newparticle.setVertex         (mcGenParticle.getVertex());
        newparticle.setEndpoint       (mcGenParticle.getEndpoint());
        newparticle.setMomentum       (mcGenParticle.getMomentum());
        newparticle.setMomentumAtEndpoint(mcGenParticle.getMomentumAtEndpoint());
        newparticle.setSpin           (mcGenParticle.getSpin());
        newparticle.setColorFlow      (mcGenParticle.getColorFlow());

    }

    size_t n = mcGenCol->size();
    for (size_t i = 0; i < n; ++i) {
        auto particle_gen = mcGenCol->at(i);
        auto particle_sim = mcCol->at(i);

        // Add parent-daughter relation
        for(auto imc = 0; imc<particle_gen.parents_size(); imc++){
            auto colidx = particle_gen.getParents(imc).getObjectID().index;
            particle_sim.addToParents(mcCol->at(colidx));
        }
        for(auto imc = 0; imc<particle_gen.daughters_size(); imc++){
            auto colidx = particle_gen.getDaughters(imc).getObjectID().index;
            particle_sim.addToDaughters(mcCol->at(colidx));
        }

    }
    
    msg() << "mcCol size (original) : " << mcCol->size() << endmsg;

    // reset
    m_track2primary.clear();
 
}

void
Edm4hepWriterAnaElemTool::EndOfEventAction(const G4Event* anEvent) {

    msg() << "mcCol size (after simulation) : " << mcCol->size() << endmsg;

    // Note about this part: DD4hep readout name is same to the collection name in Geant4.
    // However, some collections are created in Geant4 only, which should be also saved into EDM4hep.
    // When save the collections to EDM4hep, we keep the same name as Geant4.

    // readout defined in DD4hep
    auto lcdd = &(dd4hep::Detector::getInstance());
    auto allReadouts = lcdd->readouts();

    std::map<std::string, edm4hep::SimTrackerHitCollection*> _simTrackerHitColMap;
    std::map<std::string, edm4hep::SimCalorimeterHitCollection*> _simCalorimeterHitColMap;
    std::map<std::string, edm4hep::CaloHitContributionCollection*> _caloHitContribColMap;

    // Need to create all the collections defined in DD4hep
    for (auto& readout : allReadouts) {
        const std::string& current_collection_name = readout.first;
        info() << "Readout " << current_collection_name << endmsg;

        if (m_trackerColMap.find(current_collection_name) != m_trackerColMap.end()) {
            _simTrackerHitColMap[current_collection_name] = m_trackerColMap[current_collection_name]->createAndPut();
        } else if (m_calorimeterColMap.find(current_collection_name) != m_calorimeterColMap.end()) {
            _simCalorimeterHitColMap[current_collection_name] = m_calorimeterColMap[current_collection_name]->createAndPut();
            _caloHitContribColMap[current_collection_name] = m_caloContribColMap[current_collection_name]->createAndPut();
        } else {
            warning() << "Readout " << current_collection_name << " is defined in DD4hep, but not registered in Edm4hepWriterAnaElemTool. "
                      << "Please register it in m_trackerColNames or m_calorimeterColNames. " << endmsg;
            continue;
        }
    }
    // Also need to create the collections only defined in Geant4
    for (auto& _name: m_trackerColNames) {
        std::string col_name = _name + "Collection";
        if (_simTrackerHitColMap.find(col_name) == _simTrackerHitColMap.end()) {
            info() << "Additional Readout " << col_name << endmsg;
            _simTrackerHitColMap[col_name] = m_trackerColMap[col_name]->createAndPut();
        }
    }
    for (auto& _name: m_calorimeterColNames) {
        std::string col_name = _name + "Collection";
        if (_simCalorimeterHitColMap.find(col_name) == _simCalorimeterHitColMap.end()) {
            info() << "Additional Readout " << col_name << endmsg;
            _simCalorimeterHitColMap[col_name] = m_calorimeterColMap[col_name]->createAndPut();
            _caloHitContribColMap[col_name] = m_caloContribColMap[col_name]->createAndPut();
        }
    }

    // retrieve the hit collections
    G4HCofThisEvent* collections = anEvent->GetHCofThisEvent();
    if (!collections) {
        warning() << "No collections found. " << endmsg;
        return;
    }
    int Ncol = collections->GetNumberOfCollections();
    for (int icol = 0; icol < Ncol; ++icol) {
        G4VHitsCollection* collect = collections->GetHC(icol);
        if (!collect) {
            warning() << "Collection iCol " << icol << " is missing" << endmsg;
            continue;
        }
        size_t nhits = collect->GetSize();
        info() << "Collection " << collect->GetName()
               << " #" << icol
               << " has " << nhits << " hits."
               << endmsg;
        if (nhits==0) {
            // just skip this collection.
            continue;
        }

        edm4hep::SimTrackerHitCollection* tracker_col_ptr = nullptr;
        edm4hep::SimCalorimeterHitCollection* calo_col_ptr = nullptr;
        edm4hep::CaloHitContributionCollection* calo_contrib_col_ptr = nullptr;

        const std::string& current_collection_name = collect->GetName();
        // the mapping between hit collection and the data handler
        if (_simTrackerHitColMap.find(current_collection_name) != _simTrackerHitColMap.end()) {
            tracker_col_ptr = _simTrackerHitColMap[current_collection_name];
        } else if (_simCalorimeterHitColMap.find(current_collection_name) != _simCalorimeterHitColMap.end()) {
            calo_col_ptr = _simCalorimeterHitColMap[current_collection_name];
            calo_contrib_col_ptr = _caloHitContribColMap[current_collection_name];
        } else {
            warning() << "Collection name: " << current_collection_name << " is in Geant4, but not registered in Edm4hepWriterAnaElemTool. " 
                      << "Please register in Edm4hepWriterAnaElemTool. " << endmsg;
            continue;
        }

        // There are different types (new and old)

        dd4hep::sim::Geant4HitCollection* coll = dynamic_cast<dd4hep::sim::Geant4HitCollection*>(collect);
        if (coll) {
            info() << " cast to dd4hep::sim::Geant4HitCollection. " << endmsg;
            for(size_t i=0; i<nhits; ++i)   {

                dd4hep::sim::Geant4HitData* h = coll->hit(i);

                dd4hep::sim::Geant4Tracker::Hit* trk_hit = dynamic_cast<dd4hep::sim::Geant4Tracker::Hit*>(h);
                if ( 0 != trk_hit )   {
                    dd4hep::sim::Geant4HitData::Contribution& t = trk_hit->truth;
                    int trackID = t.trackID;
                    // t.trackID = m_truth->particleID(trackID);
                }
                // Geant4Calorimeter::Hit* cal_hit = dynamic_cast<Geant4Calorimeter::Hit*>(h);
                // if ( 0 != cal_hit )   {
                //     Geant4HitData::Contributions& c = cal_hit->truth;
                //     for(Geant4HitData::Contributions::iterator j=c.begin(); j!=c.end(); ++j)  {
                //         Geant4HitData::Contribution& t = *j;
                //         int trackID = t.trackID;
                //         // t.trackID = m_truth->particleID(trackID);
                //     }
                // }
            }
            continue;
        }

        typedef G4THitsCollection<dd4hep::sim::Geant4Hit> HitCollection;
        HitCollection* coll2 = dynamic_cast<HitCollection*>(collect);

        if (coll2) {
            info() << " cast to G4THitsCollection<dd4hep::sim::Geant4Hit>. " << endmsg;

            int n_trk_hit = 0;
            int n_cal_hit = 0;

            for(size_t i=0; i<nhits; ++i)   {
                dd4hep::sim::Geant4Hit* h = dynamic_cast<dd4hep::sim::Geant4Hit*>(coll2->GetHit(i));
                if (!h) {
                    warning() << "Failed to cast to dd4hep::sim::Geant4Hit. " << endmsg;
                    continue;
                }

                dd4hep::sim::Geant4TrackerHit* trk_hit = dynamic_cast<dd4hep::sim::Geant4TrackerHit*>(h);
                if (trk_hit && tracker_col_ptr) {
                    ++n_trk_hit;
                    // auto edm_trk_hit = trackercols->create();
                    auto edm_trk_hit = tracker_col_ptr->create();

                    edm_trk_hit.setCellID(trk_hit->cellID);
                    edm_trk_hit.setEDep(trk_hit->energyDeposit/CLHEP::GeV);
                    edm_trk_hit.setTime(trk_hit->truth.time/CLHEP::ns);
                    edm_trk_hit.setPathLength(trk_hit->length/CLHEP::mm);
                    // lc_hit->setMCParticle(lc_mcp);
                    double pos[3] = {trk_hit->position.x()/CLHEP::mm,
                                     trk_hit->position.y()/CLHEP::mm,
                                     trk_hit->position.z()/CLHEP::mm};
                    edm_trk_hit.setPosition(edm4hep::Vector3d(pos));

                    float mom[3] = {trk_hit->momentum.x()/CLHEP::GeV,
                                    trk_hit->momentum.y()/CLHEP::GeV,
                                    trk_hit->momentum.z()/CLHEP::GeV};
                    edm_trk_hit.setMomentum(edm4hep::Vector3f(mom));

                    // get the truth or contribution
                    auto& truth = trk_hit->truth;
                    int trackID = truth.trackID;
                    
                    int pritrkid = m_track2primary[trackID];
                    if (pritrkid <= 0) {
                        error() << "Failed to find the primary track for trackID #" << trackID << endmsg;
                        pritrkid = 1;
                    }

                    if (m_userinfo) {
                        auto idxedm4hep =  m_userinfo->idxG4Track2Edm4hep(pritrkid);
                        if (idxedm4hep != -1) {
                            edm_trk_hit.setMCParticle(mcCol->at(idxedm4hep));
                        }
                    }

                    if (pritrkid != trackID) {
                        // If the track is a secondary, then the primary track id and current track id is different
                        edm_trk_hit.setProducedBySecondary(true);
                    }
                }

                dd4hep::sim::Geant4CalorimeterHit* cal_hit = dynamic_cast<dd4hep::sim::Geant4CalorimeterHit*>(h);
                if (cal_hit && calo_col_ptr) {
                    ++n_cal_hit;
                    auto edm_calo_hit = calo_col_ptr->create();
                    edm_calo_hit.setCellID(cal_hit->cellID);
                    edm_calo_hit.setEnergy(cal_hit->energyDeposit/CLHEP::GeV);
                    float pos[3] = {cal_hit->position.x()/CLHEP::mm,
                                    cal_hit->position.y()/CLHEP::mm,
                                    cal_hit->position.z()/CLHEP::mm};
                    edm_calo_hit.setPosition(edm4hep::Vector3f(pos));

                    // contribution
                    typedef dd4hep::sim::Geant4CalorimeterHit::Contributions Contributions;
                    typedef dd4hep::sim::Geant4CalorimeterHit::Contribution Contribution;
                    for (Contributions::const_iterator j = cal_hit->truth.begin();
                         j != cal_hit->truth.end(); ++j) {
                        const Contribution& c = *j;
                        // The legacy Hit object does not contains positions of contributions.
                        // float contrib_pos[] = {float(c.x/mm), float(c.y/mm), float(c.z/mm)};
                        auto edm_calo_contrib = calo_contrib_col_ptr->create();
                        edm_calo_contrib.setPDG(c.pdgID);
                        edm_calo_contrib.setEnergy(c.deposit/CLHEP::GeV);
                        edm_calo_contrib.setTime(c.time/CLHEP::ns);
                        edm_calo_contrib.setStepPosition(edm4hep::Vector3f(pos));

                        // from the track id, get the primary track
                        int pritrkid = m_track2primary[c.trackID];
                        if (pritrkid<=0) {
                            error() << "Failed to find the primary track for trackID #" << c.trackID << endmsg;
                            pritrkid = 1;
                        }

                        if (m_userinfo) {
                            auto idxedm4hep =  m_userinfo->idxG4Track2Edm4hep(pritrkid);
                            if (idxedm4hep != -1) {
                                edm_calo_contrib.setParticle(mcCol->at(idxedm4hep)); // todo
                            }
                        }
                        edm_calo_hit.addToContributions(edm_calo_contrib);
                    }
                }

            }

            info() << n_trk_hit << " hits cast to dd4hep::sim::Geant4TrackerHit. " << endmsg;
            info() << n_cal_hit << " hits cast to dd4hep::sim::Geant4CalorimeterHit. " << endmsg;


            continue;
        }

        warning() << "Failed to convert to collection "
                  << collect->GetName()
                  << endmsg;
        
    }
}

void
Edm4hepWriterAnaElemTool::PreUserTrackingAction(const G4Track* track) {
    int curtrkid = track->GetTrackID();
    int curparid = track->GetParentID();
    int pritrkid = curparid;

    // try to find the primary track id from the parent track id.
    if (curparid) {
        auto it = m_track2primary.find(curparid);
        if (it == m_track2primary.end()) {
            error() << "Failed to find primary track for track id " << curparid << endmsg;
        } else {
            pritrkid = it->second;
        }
    } else {
        // curparid is 0, it is primary
        pritrkid = curtrkid;
    }


    m_track2primary[curtrkid] = pritrkid;
}

void
Edm4hepWriterAnaElemTool::PostUserTrackingAction(const G4Track* track) {
    // 
    // cache all the necessary processes
    // 
    static G4VProcess* proc_decay = nullptr;
    static G4VProcess* photon_general = nullptr; // default
    static G4VProcess* photon_conv = nullptr;  // "/process/em/UseGeneralProcess false"
    if (!proc_decay || (!(photon_general || photon_conv))) {
        G4ProcessManager* pm 
            = track->GetDefinition()->GetProcessManager();
        G4int nprocesses = pm->GetProcessListLength();
        G4ProcessVector* pv = pm->GetProcessList();
            
        for(G4int i=0; i<nprocesses; ++i){
            info() << " process: " << (*pv)[i]->GetProcessName() << endmsg;
            if((*pv)[i]->GetProcessName()=="Decay"){
                proc_decay = (*pv)[i];
            } else if((*pv)[i]->GetProcessName()=="GammaGeneralProc"){
                photon_general = (*pv)[i];
                photon_conv = dynamic_cast<G4GammaGeneralProcess*>(photon_general)->GetEmProcess("conv");
                info() << "gamma general proc: " << photon_general
                       << " conv: " << photon_conv
                       << endmsg;
            } else if((*pv)[i]->GetProcessName()=="conv"){
                photon_conv = (*pv)[i];
            }
        }

    }



    int curtrkid = track->GetTrackID(); // starts from 1
    int curparid = track->GetParentID();
    int idxedm4hep = -1;

    // =========================================================================
    // note: only the primary track or the interested secondary track can
    //       get the idxedm4hep from Event or Track level User Info.
    // =========================================================================
    if (m_userinfo) {
        idxedm4hep =  m_userinfo->idxG4Track2Edm4hep(curtrkid);
    }

    if (idxedm4hep == -1) {
        auto trackinfo = dynamic_cast<CommonUserTrackInfo*>(track->GetUserInformation());
        if (trackinfo) {
            idxedm4hep = trackinfo->idxEdm4hep();
        }
    }
    
    // =========================================================================
    // if there is no pointer to MCParticle collection, skip this track.
    // =========================================================================
    if (idxedm4hep == -1) {
        return;
    }

    // =========================================================================
    // first, update the track self
    // =========================================================================
    auto primary_particle = mcCol->at(idxedm4hep);

    const G4ThreeVector& stop_pos  = track->GetPosition();
    edm4hep::Vector3d endpoint(stop_pos.x()/CLHEP::mm,
                               stop_pos.y()/CLHEP::mm,
                               stop_pos.z()/CLHEP::mm);
    primary_particle.setEndpoint(endpoint);

    const G4ThreeVector& stop_mom = track->GetMomentum();

    edm4hep::Vector3f mom_endpoint(stop_mom.x()/CLHEP::GeV,
                                   stop_mom.y()/CLHEP::GeV,
                                   stop_mom.z()/CLHEP::GeV);
    primary_particle.setMomentumAtEndpoint(mom_endpoint);


    // =========================================================================
    // then, create secondary track if necessary
    // =========================================================================

    G4TrackingManager* tm = G4EventManager::GetEventManager() 
        -> GetTrackingManager();
    G4TrackVector* secondaries = tm->GimmeSecondaries();
    if(!secondaries) {
        return;
    }

    // ===================================================================
    // Update the flags of the primary particles
    // ===================================================================

    // flags
    bool is_decay = false;

    size_t nSeco = secondaries->size();
    for (size_t i = 0; i < nSeco; ++i) {
        G4Track* sectrk = (*secondaries)[i];
        G4ParticleDefinition* secparticle = sectrk->GetDefinition();
        const G4VProcess* creatorProcess = sectrk->GetCreatorProcess();

        double Ek = sectrk->GetKineticEnergy();
        const auto& pos = sectrk->GetPosition();
                
        // select the necessary processes
        if (creatorProcess==proc_decay || creatorProcess==photon_general || creatorProcess==photon_conv) {

        } else if (Ek >= m_sectrk_Ek.value() && fabs(pos.rho()) < m_sectrk_rho.value() && fabs(pos.z()) < m_sectrk_z.value()) {
            // if the Ek > x 
        } else {
            // skip this secondary track
            continue;
        }
                
        debug() << "Creator Process is " << creatorProcess->GetProcessName()
                << " for secondary particle: "
                << " idx: " << i
                << " trkid: " << sectrk->GetTrackID() // not valid until track
                << " particle: " << secparticle->GetParticleName()
                << " pdg: " << secparticle->GetPDGEncoding()
                << " at position: " << sectrk->GetPosition() //
                << " time: " << sectrk->GetGlobalTime()
                << " momentum: " << sectrk->GetMomentum() // 
                << endmsg;

        if (creatorProcess==proc_decay) is_decay = true;

        // create secondaries in MC particles
        // todo: convert the const collection to non-const
        // auto mcCol = const_cast<edm4hep::MCParticleCollection*>(m_mcParCol.get());
        auto mcp = mcCol->create();
        mcp.setPDG(secparticle->GetPDGEncoding());
        mcp.setGeneratorStatus(0); // not created by Generator
        mcp.setCreatedInSimulation(1);
        mcp.setCharge(secparticle->GetPDGCharge());
        mcp.setTime(sectrk->GetGlobalTime()/CLHEP::ns); // todo
        mcp.setMass(secparticle->GetPDGMass()/CLHEP::GeV); // EDM4hep unit: GeV

        const G4ThreeVector& sec_init_pos = sectrk->GetPosition();
        double x=sec_init_pos.x()/CLHEP::mm;
        double y=sec_init_pos.y()/CLHEP::mm;
        double z=sec_init_pos.z()/CLHEP::mm;

        const G4ThreeVector& sec_init_mom = sectrk->GetMomentum();
        double px=sec_init_mom.x()/CLHEP::GeV;
        double py=sec_init_mom.y()/CLHEP::GeV;
        double pz=sec_init_mom.z()/CLHEP::GeV;
        mcp.setVertex(edm4hep::Vector3d(x,y,z)); // todo
        mcp.setEndpoint(edm4hep::Vector3d(x,y,z)); // todo
        mcp.setMomentum(edm4hep::Vector3f(px,py,pz)); // todo
        mcp.setMomentumAtEndpoint(edm4hep::Vector3f(px,py,pz)); //todo

        mcp.addToParents(primary_particle);
        primary_particle.addToDaughters(mcp);

        // store the edm4hep obj idx in track info.
        // using this idx, the MCParticle object could be modified later. 
        auto trackinfo = new CommonUserTrackInfo();
        trackinfo->setIdxEdm4hep(mcp.getObjectID().index);
        sectrk->SetUserInformation(trackinfo);
        debug() << " Appending MCParticle: (id: " 
                << mcp.getObjectID().index << ")"
                << endmsg;
    }

    // now update
    if (is_decay) {
        primary_particle.setDecayedInTracker(is_decay);
        primary_particle.setDecayedInCalorimeter(is_decay);
    }

}

void
Edm4hepWriterAnaElemTool::UserSteppingAction(const G4Step* aStep) {
    auto aTrack = aStep->GetTrack();
    // try to get user track info
    auto trackinfo = dynamic_cast<CommonUserTrackInfo*>(aTrack->GetUserInformation());

    // ========================================================================
    // Note:
    // if there is no track info, then do nothing. 
    // ========================================================================
    if (trackinfo) {
        // back scattering is defined as following:
        // - pre point is not in tracker
        // - post point is in tracker
        // For details, look at Mokka's source code:
        //   https://llrforge.in2p3.fr/trac/Mokka/browser/trunk/source/Kernel/src/SteppingAction.cc
        const auto& pre_pos = aStep->GetPreStepPoint()->GetPosition();
        const auto& post_pos = aStep->GetPostStepPoint()->GetPosition();

        bool is_pre_in_tracker = pre_pos.perp() < R && std::fabs(pre_pos.z()) < Z;
        bool is_post_in_tracker = post_pos.perp() < R && std::fabs(post_pos.z()) < Z;

        if ((!is_pre_in_tracker) and is_post_in_tracker) {
            // set back scattering
            auto idxedm4hep = trackinfo->idxEdm4hep();
            mcCol->at(idxedm4hep).setBackscatter(true);
            debug() << " set back scatter for MCParticle "
                    << " (ID: " << idxedm4hep << ")"
                    << endmsg;
        }
    }
}

StatusCode
Edm4hepWriterAnaElemTool::initialize() {
    StatusCode sc;

    // note: 
    // the name is without the collection suffix
    // Convention:
    // - VXD: VXDCollection
    // - EcalBarrel: EcalBarrelCollection + EcalBarrelContributionCollection

    // SimTrackerHitCollections
    for (const auto& name : m_trackerColNames) {
        std::string name_col = name + "Collection";
        auto col = new DataHandle<edm4hep::SimTrackerHitCollection>(name_col, Gaudi::DataHandle::Writer, this);
        m_trackerColMap[name_col] = col;
    }

    // SimCalorimeterHitCollections
    for (const auto& name : m_calorimeterColNames) {
        std::string name_col = name + "Collection";
        std::string name_contrib_col = name + "ContributionCollection";
        auto col = new DataHandle<edm4hep::SimCalorimeterHitCollection>(name_col, Gaudi::DataHandle::Writer, this);
        m_calorimeterColMap[name_col] = col;
        auto col_contrib = new DataHandle<edm4hep::CaloHitContributionCollection>(name_contrib_col, Gaudi::DataHandle::Writer, this);
        m_caloContribColMap[name_col] = col_contrib;
    }

    return sc;
}

StatusCode
Edm4hepWriterAnaElemTool::finalize() {
    StatusCode sc;

    return sc;
}


