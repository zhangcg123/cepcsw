#include "DataHelper/Navigation.h"

#include "edm4hep/SimTrackerHit.h"
#include "edm4hep/TrackerHit.h"

Navigation* Navigation::m_fNavigation = nullptr;

Navigation* Navigation::Instance(){
  if(!m_fNavigation) m_fNavigation = new Navigation();
  return m_fNavigation;
}

Navigation::Navigation(){
}

Navigation::~Navigation(){
}

void Navigation::Initialize(){
  m_hitColVec.clear();
  m_assColVec.clear();
  for(std::map<int, edm4hep::TrackerHit>::iterator it=m_trkHits.begin();it!=m_trkHits.end();it++){
    // delete it->second;
  }
  m_trkHits.clear();
}

#if EDM4HEP_BUILD_VERSION <= EDM4HEP_VERSION(0, 10, 5)
std::optional<edm4hep::TrackerHit> Navigation::GetTrackerHit(const edm4hep::ObjectID& obj_id, bool delete_by_caller){
#else
std::optional<edm4hep::TrackerHit> Navigation::GetTrackerHit(const podio::ObjectID& obj_id, bool delete_by_caller){
#endif
  int id = obj_id.collectionID * 10000000 + obj_id.index;
  if(!delete_by_caller){
    auto it = m_trkHits.find(id);
    if(it!=m_trkHits.end()) return it->second;
  }
  /*
  for(int i=0;i<m_assColVec.size();i++){
    for(auto ass : *m_assColVec[i]){
      auto rec_id = ass.getRec().getObjectID();
      if(rec_id.collectionID!=id.collectionID)break;
      else if(rec_id.index==id.index){
	m_trkHits.push_back(ass.getRec());
	return &(m_trkHits.back());
      }
    }
  }
  */
  for(int i=0;i<m_hitColVec.size();i++){
    for(auto hit : *m_hitColVec[i]){
      auto this_id = hit.getObjectID();
      if(this_id.collectionID!=obj_id.collectionID)break;
      else if(this_id.index==obj_id.index){
        edm4hep::TrackerHit  hit_iterface = hit;
	if(!delete_by_caller) m_trkHits.insert_or_assign(id, hit_iterface);
	return hit_iterface;//&(m_trkHits[id]);
      }
    }
  }
  
  throw std::runtime_error("Not found TrackerHit");
  return std::nullopt;
}

#if EDM4HEP_BUILD_VERSION <= EDM4HEP_VERSION(0, 10, 5)
std::vector<edm4hep::SimTrackerHit> Navigation::GetRelatedTrackerHit(const edm4hep::ObjectID& id){
#else
std::vector<edm4hep::SimTrackerHit> Navigation::GetRelatedTrackerHit(const podio::ObjectID& id){
#endif
  std::vector<edm4hep::SimTrackerHit> hits;
  for(int i=0;i<m_assColVec.size();i++){
    for(auto ass : *m_assColVec[i]){
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
        auto recoHit = ass.getFrom();
        auto simHit  = ass.getTo();
#else
        auto recoHit = ass.getRec();
        auto simHit  = ass.getSim();
#endif
        
        auto this_id = recoHit.getObjectID();
        if(this_id.collectionID!=id.collectionID) {
            break;
        } else if(this_id.index==id.index) {
            hits.push_back(simHit);
        }
    }
  }
  return hits;
}

std::vector<edm4hep::SimTrackerHit> Navigation::GetRelatedTrackerHit(const edm4hep::TrackerHit& hit){
  std::vector<edm4hep::SimTrackerHit> hits;
  for(int i=0;i<m_assColVec.size();i++){
    for(auto ass : *m_assColVec[i]){
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
        auto recoHit = ass.getFrom();
        auto simHit  = ass.getTo();
#else
        auto recoHit = ass.getRec();
        auto simHit  = ass.getSim();
#endif
        
        if(recoHit.getObjectID().collectionID != hit.getObjectID().collectionID) {
            break;
        } else if(recoHit==hit) {
            hits.push_back(simHit);
        }
    }
  }
  return hits;
}

std::vector<edm4hep::SimTrackerHit> Navigation::GetRelatedTrackerHit(const edm4hep::TrackerHit& hit, const Navigation::CEPCSWTrackerHitSimTrackerHitLinkCollection * col){
  std::vector<edm4hep::SimTrackerHit> hits;
  for(auto ass : *col){
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
      auto recoHit = ass.getFrom();
      auto simHit  = ass.getTo();
#else
      auto recoHit = ass.getRec();
      auto simHit  = ass.getSim();
#endif
      
      if(recoHit.getObjectID().collectionID != hit.getObjectID().collectionID) {
          break;
      } else if(recoHit==hit) {
          hits.push_back(simHit);
      }
  }
  return hits;
}
