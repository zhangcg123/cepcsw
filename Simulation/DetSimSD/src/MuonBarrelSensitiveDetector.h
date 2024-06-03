
#ifndef MuonBarrelSensitiveDetector_h
#define MuonBarrelSensitiveDetector_h

#include "DetSimSD/DDG4SensitiveDetector.h"
#include "DDG4/Defs.h"

class MuonBarrelSensitiveDetector: public DDG4SensitiveDetector {
 public:
  MuonBarrelSensitiveDetector(const std::string& name, dd4hep::Detector& description);
  
  void Initialize(G4HCofThisEvent* HCE);
  G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
  void EndOfEvent(G4HCofThisEvent* HCE);
  
 protected:

  HitCollection* m_hc = nullptr;
  
};
#endif
