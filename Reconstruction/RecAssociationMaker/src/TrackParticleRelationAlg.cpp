#include "TrackParticleRelationAlg.h"

DECLARE_COMPONENT(TrackParticleRelationAlg)

TrackParticleRelationAlg::TrackParticleRelationAlg(const std::string& name, ISvcLocator* svcLoc)
: GaudiAlgorithm(name, svcLoc){
  declareProperty("MCParticleCollection", m_inMCParticleColHdl, "Handle of the Input MCParticle collection");
}

StatusCode TrackParticleRelationAlg::initialize() {
  for(auto name : m_inTrackCollectionNames) {
    m_inTrackColHdls.push_back(new DataHandle<edm4hep::TrackCollection> (name, Gaudi::DataHandle::Reader, this));
    m_outColHdls.push_back(new DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> (name+"ParticleAssociation", Gaudi::DataHandle::Writer, this));
  }
  
  for(unsigned i=0; i<m_inAssociationCollectionNames.size(); i++) {
    m_inAssociationColHdls.push_back(new DataHandle<edm4hep::MCRecoTrackerAssociationCollection> (m_inAssociationCollectionNames[i], Gaudi::DataHandle::Reader, this));
  }
  
  if(m_inAssociationColHdls.size()==0) {
    warning() << "no Association Collection input" << endmsg;
    return StatusCode::FAILURE;
  }

  m_nEvt = 0;
  return GaudiAlgorithm::initialize();
}

StatusCode TrackParticleRelationAlg::execute() {
  info() << "start Event " << m_nEvt << endmsg;
  
  // MCParticle
  const edm4hep::MCParticleCollection* mcpCol = nullptr;
  try {
    mcpCol = m_inMCParticleColHdl.get();
    for (auto mcp : *mcpCol) {
      debug() << "id=" << mcp.id() << " pdgid=" << mcp.getPDG() << " charge=" << mcp.getCharge() << " genstat=" << mcp.getGeneratorStatus() << " simstat=" << mcp.getSimulatorStatus()
	      << " p=" << mcp.getMomentum() << endmsg;
      int nparents = mcp.parents_size();
      int ndaughters = mcp.daughters_size();
      for (int i=0; i<nparents; i++) {
	debug() << "<<<<<<" << mcp.getParents(i).id() << endmsg;
      }
      for (int i=0; i<ndaughters; i++) {
        debug() << "<<<<<<" << mcp.getDaughters(i).id() << endmsg;
      }
    }
  }
  catch ( GaudiException &e ) {
    debug() << "Collection " << m_inMCParticleColHdl.fullKey() << " is unavailable in event " << m_nEvt << endmsg;
  }
  if(!mcpCol) {
    warning() << "pass this event because MCParticle collection can not read" << endmsg; 
    return StatusCode::SUCCESS;
  }

  // Prepare map from hit to MCParticle
  std::map<edm4hep::TrackerHit, edm4hep::MCParticle> mapHitParticle;
  debug() << "reading Association" << endmsg;
  for (auto hdl : m_inAssociationColHdls) {
    const edm4hep::MCRecoTrackerAssociationCollection* assCol = nullptr;
    try {
      assCol = hdl->get();
    }
    catch ( GaudiException &e ) {
      debug() << "Collection " << hdl->fullKey() << " is unavailable in event " << m_nEvt << endmsg;
      return StatusCode::FAILURE;
    }
    for (auto ass: *assCol) {
      auto hit = ass.getRec();
      auto particle = ass.getSim().getMCParticle();
      mapHitParticle[hit] = particle;
    }
  }
  
  // Handle all input TrackCollection
  for (unsigned icol=0; icol<m_inTrackColHdls.size(); icol++) {
    auto hdl = m_inTrackColHdls[icol];

    const edm4hep::TrackCollection* trkCol = nullptr;
    try {
      trkCol = hdl->get();
    }
    catch ( GaudiException &e ) {
      debug() << "Collection " << hdl->fullKey() << " is unavailable in event " << m_nEvt << endmsg;
    }

    auto outCol = m_outColHdls[icol]->createAndPut();
    if(!outCol) {
      error() << "Collection " << m_outColHdls[icol]->fullKey() << " cannot be created" << endmsg;
      return StatusCode::FAILURE;
    }
    
    if(trkCol) {
      std::map<edm4hep::MCParticle, std::vector<edm4hep::TrackerHit> > mapParticleHits;

      for (auto track: *trkCol) {
	std::map<edm4hep::MCParticle, int> mapParticleNHits;
	
	// Count hits related to MCParticle
	int nhits = track.trackerHits_size();
	for (int ihit=0; ihit<nhits; ihit++) {
	  auto hit = track.getTrackerHits(ihit);
	  auto itHP  = mapHitParticle.find(hit);
	  if (itHP==mapHitParticle.end()) {
	    warning() << "track " << track.id() << "'s hit " << hit.id() << "not in association list, will be ignored" << endmsg;
	  }
	  else {
	    auto particle = itHP->second;
	    if(!particle.isAvailable()) continue;
	    debug() << "track " << track.id() << "'s hit " << hit.id() << " link to MCParticle " << particle.id() << endmsg;
	    auto itPN = mapParticleNHits.find(particle);
	    if (itPN!=mapParticleNHits.end()) itPN->second++;
	    else                             mapParticleNHits[particle] = 1;

	    if (msgLevel(MSG::DEBUG)) mapParticleHits[particle].push_back(hit);
	  }
	}
	debug() << "Total " << mapParticleNHits.size() << " particles link to the track " << track.id() << endmsg;

	// Save to collection
	for (std::map<edm4hep::MCParticle, int>::iterator it=mapParticleNHits.begin(); it!=mapParticleNHits.end(); it++) {
	  auto ass = outCol->create();
	  ass.setWeight(it->second);
	  ass.setRec(track);
	  ass.setSim(it->first);
	}
      }
      
      if (msgLevel(MSG::DEBUG)) {
	for (std::map<edm4hep::MCParticle, std::vector<edm4hep::TrackerHit> >::iterator it=mapParticleHits.begin(); it!=mapParticleHits.end(); it++) {
	  auto particle = it->first;
	  auto hits     = it->second;
	  debug() << "=== MCPaticle ===" << particle << endmsg;
	  for (auto hit : hits) {
	    debug() << hit << endmsg;
	  }
	}
      }
    }
  }

  m_nEvt++;
  return StatusCode::SUCCESS;
}

StatusCode TrackParticleRelationAlg::finalize() {
  
  return StatusCode::SUCCESS;
}
