#include "MuonEndcapSensitiveDetector.h"

#include "G4Step.hh"
#include "G4VProcess.hh"
//#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
//#include "UserTrackInformation.hh"
#include "DD4hep/DD4hepUnits.h"

MuonEndcapSensitiveDetector::MuonEndcapSensitiveDetector(const std::string& name,
								 dd4hep::Detector& description)
  : DDG4SensitiveDetector(name, description),
    m_hc(nullptr){
  
  G4String CollName=name+"Collection";
  collectionName.insert(CollName);
}

void MuonEndcapSensitiveDetector::Initialize(G4HCofThisEvent* HCE){
  m_hc = new HitCollection(GetName(), collectionName[0]);
  int HCID = G4SDManager::GetSDMpointer()->GetCollectionID(m_hc);
  HCE->AddHitsCollection( HCID, m_hc ); 
}

G4bool MuonEndcapSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*){
  G4TouchableHandle touchPost = step->GetPostStepPoint()->GetTouchableHandle(); 
  G4TouchableHandle touchPre  = step->GetPreStepPoint()->GetTouchableHandle();
  auto currentTrack = step->GetTrack();
  auto pname = currentTrack->GetParticleDefinition()->GetParticleName();
  auto touchable = step->GetPreStepPoint()->GetTouchable();
  auto physical = touchable->GetVolume();
/*  if ( physical->GetName() == "surface" )
  {
      if ( pname == "opticalphoton" )
      {
        G4double random = G4UniformRand();
        if(random < 0.1)
        {
          currentTrack->SetTrackStatus(fStopAndKill);
        }
      }
  }*/ 
  dd4hep::sim::Geant4StepHandler h(step);
  if (fabs(h.trackDef()->GetPDGCharge()) < 0.01) return true;

  dd4hep::Position prePos    = h.prePos();
  dd4hep::Position postPos   = h.postPos();
  dd4hep::Position direction = postPos - prePos;
  dd4hep::Position position  = mean_direction(prePos,postPos);
  double   hit_len   = direction.R();
  if (hit_len < 1E-9) return true;
  if (hit_len > 0) {
    //*2, mean_length divided 2 twice, which inherited the mistakes from $DD4hep/Object.cpp mean_length function
    double new_len = mean_length(h.preMom(),h.postMom())*2;
    direction *= new_len/hit_len;
  }

  dd4hep::sim::Geant4TrackerHit* hit = nullptr;
  hit = new dd4hep::sim::Geant4TrackerHit(h.track->GetTrackID(),
					  h.track->GetDefinition()->GetPDGEncoding(),
					  step->GetTotalEnergyDeposit(),
					  h.track->GetGlobalTime());

  if ( hit )  {
    hit->cellID  = getCellID( step ) ;
    hit->energyDeposit =  step->GetTotalEnergyDeposit();
    hit->position = position;
    hit->momentum = direction;
    hit->length   = hit_len;
    m_hc->insert(hit);
    if ( physical->GetName() == "SiPM" )
    {
      currentTrack->SetTrackStatus(fStopAndKill);
    }
    return true;
  }
  throw std::runtime_error("new() failed: Cannot allocate hit object");
  return false;
}

void MuonEndcapSensitiveDetector::EndOfEvent(G4HCofThisEvent* HCE){
}
