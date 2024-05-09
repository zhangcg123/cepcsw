#include "ExampleAnaDoseElemTool.h"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4THitsCollection.hh"
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

#include "DD4hep/Detector.h"
#include "DD4hep/Plugins.h"
#include "DDG4/Geant4Converter.h"
#include "DDG4/Geant4Mapping.h"
#include "DDG4/Geant4HitCollection.h"
#include "DDG4/Geant4Data.h"
#include "DetSimSD/Geant4Hits.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

DECLARE_COMPONENT(ExampleAnaDoseElemTool)

void ExampleAnaDoseElemTool::Preparecoeff(const G4ThreeVector inputnbins, const G4ThreeVector inputcoormin, const G4ThreeVector inputcoormax, const G4ThreeVector inputregionhist, const std::string path, const G4double inputhehadcut)
{
  //  nbins = G4ThreeVector(20,20,20);
  //  //  coormin = G4ThreeVector(-1000.,-1000.,-1000.);
  //  //  coormax = G4ThreeVector(1000.,1000.,1000.);
  //  //  regionhist = G4ThreeVector(26,1.e-14,0.02);
  nbins = inputnbins;
  regionhist = inputregionhist;
  coormin = inputcoormin;
  coormax = inputcoormax;
  vecwidth =G4ThreeVector((coormax[0]-coormin[0])/nbins[0], (coormax[1]-coormin[1])/nbins[1], (coormax[2]-coormin[2])/nbins[2]);
  hehadcut = inputhehadcut;
  G4cout<< "in Run.cc: begin gen vector..."<<G4endl;
  G4cout << "Nbins: " <<nbins<<coormin<<coormax<<regionhist<<vecwidth<<G4endl;
  G4cout << "Dose coeff. files are in "<<path<<G4endl;
  G4cout << "High energy hadron cut: "<<hehadcut<<G4endl;

  Readcoffe(path, "n_GeV_icru95joint.txt",           nbinscoeffdoseeqneu,          coeffneu);
  Readcoffe(path, "photon_GeV_icru95joint.txt",      nbinscoeffdoseeqphoton,       coeffproton);
  Readcoffe(path, "heion_GeV_icru95joint.txt",       nbinscoeffdoseeqhe,           coeffele);
  Readcoffe(path, "proton_GeV_icru95joint.txt",      nbinscoeffdoseeqproton,       coeffpos);
  Readcoffe(path, "electron_GeV_icru95joint.txt",    nbinscoeffdoseeqele,          coeffhe);
  Readcoffe(path, "positron_GeV_icru95joint.txt",    nbinscoeffdoseeqpos,          coeffphoton);
  Readcoffe(path, "muonpositive_GeV_icru95joint.txt",nbinscoeffdoseeqmup,          coeffmup);
  Readcoffe(path, "muonnegative_GeV_icru95joint.txt",nbinscoeffdoseeqmum,          coeffmum);
  Readcoffe(path, "pionpositive_GeV_icru95joint.txt",nbinscoeffdoseeqpip,          coeffpip);
  Readcoffe(path, "pionnegative_GeV_icru95joint.txt",nbinscoeffdoseeqpim,          coeffpim);
  Readcoffe(path, "nflu_GeV_rd50.txt",               nbinscoeff1mevneueqfluenceneu,coeff1mevneueqfluenceneu);
  Readcoffe(path, "protonflu_GeV_rd50.txt",          nbinscoeff1mevneueqfluenceproton,coeff1mevneueqfluenceproton);
  Readcoffe(path, "pionflu_GeV_rd50.txt",            nbinscoeff1mevneueqfluencepion,coeff1mevneueqfluencepion);
  Readcoffe(path, "electronflu_GeV_rd50.txt",        nbinscoeff1mevneueqfluenceele,coeff1mevneueqfluenceele);

  fVector.clear();
  fnneu.clear();
  fdet1.clear();
  fair.clear();
  fdoseeqright.clear();
  f1mevnfluen.clear();
  fhehadfluen.clear();
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    (fVector).push_back(0.);
    (fnneu).push_back(0.);
    (f1mevnfluen).push_back(0.);
    (fhehadfluen).push_back(0.);
    (fdoseeqright).push_back(0.);
  }
  for (G4int i = 0;i<regionhist[0];i++){
    (fdet1).push_back(0);
    (fair).push_back(0);
  }

}

void ExampleAnaDoseElemTool::Initialize(){
  //*********************************
  // initialize Run...
  //********************************
  //  fRun = new Run();
  info() <<"Grid nbins: "<<endmsg;
  if (m_gridnbins.value().size()!=3){
    error() <<"Grid nbins size != 3" <<endmsg;
  }
  for (auto tempnbins: m_gridnbins.value()) {
    info() << tempnbins << endmsg;
  }
  info() <<"Coormin: "<<endmsg;
  if (m_coormin.value().size()!=3){
    error() <<"Coormin size != 3" <<endmsg;
  }
  for (auto tempnbins: m_coormin.value()) {
    info() << tempnbins << endmsg;
  }
  info() <<"Coormax: "<<endmsg;
  if (m_coormax.value().size()!=3){
    error() <<"Coormax size != 3" <<endmsg;
  }
  for (auto tempnbins: m_coormax.value()) {
    info() << tempnbins << endmsg;
  }
  info() <<"Regionhist: "<<endmsg;
  if (m_regionhist.value().size()!=3){
    error() <<"Regionhist size != 3" <<endmsg;
  }
  for (auto tempnbins: m_regionhist.value()) {
    info() << tempnbins << endmsg;
  }

    G4ThreeVector innbins(m_gridnbins.value()[0],m_gridnbins.value()[1],m_gridnbins.value()[2]);
    G4ThreeVector incoormin(m_coormin.value()[0],m_coormin.value()[1],m_coormin.value()[2]);
    G4ThreeVector incoormax(m_coormax.value()[0],m_coormax.value()[1],m_coormax.value()[2]);
    G4ThreeVector inregionhist(m_regionhist.value()[0],m_regionhist.value()[1],m_regionhist.value()[2]);
    Preparecoeff(innbins,incoormin,incoormax,inregionhist,m_dosecoefffilename,m_hehadcut);
    info() << "in initialize: finish gen vector ..."<<endmsg;
}

void
ExampleAnaDoseElemTool::BeginOfRunAction(const G4Run*) {
    auto msglevel = msgLevel();
    if (msglevel == MSG::VERBOSE || msglevel == MSG::DEBUG) {
        verboseOutput = true;
    }

    Initialize();
    G4cout << "Begin Run of detector simultion..." << G4endl;
}

void
ExampleAnaDoseElemTool::EndOfRunAction(const G4Run*) {
    G4cout << "End Run of detector simultion... "<<m_filename.value() << G4endl;
    Writefile(m_filename.value());
}

void
ExampleAnaDoseElemTool::BeginOfEventAction(const G4Event* anEvent) {
    msg() << "Event " << anEvent->GetEventID() << endmsg;
}

void
ExampleAnaDoseElemTool::EndOfEventAction(const G4Event* anEvent) {
    msg() << "Event " << anEvent->GetEventID() << endmsg;
    // save all data
}

void
ExampleAnaDoseElemTool::PreUserTrackingAction(const G4Track* track) {
}

void
ExampleAnaDoseElemTool::PostUserTrackingAction(const G4Track* track) {
}

void
ExampleAnaDoseElemTool::UserSteppingAction(const G4Step* aStep) {
  auto aTrack = aStep->GetTrack();
  // try to get user track info

  Setdosevalue(aStep);
}

StatusCode
ExampleAnaDoseElemTool::initialize() {
    StatusCode sc;
    
    return sc;
}

StatusCode
ExampleAnaDoseElemTool::finalize() {
    StatusCode sc;

    return sc;
}

////....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void ExampleAnaDoseElemTool::Setdosevalue(const G4Step* step)
{
  auto steppoint=step->GetPreStepPoint();
  auto prepoint=steppoint->GetPosition();
  auto postpoint=step->GetPostStepPoint()->GetPosition();
  auto momentum = steppoint->GetMomentum();
  auto momentumdirection = steppoint->GetMomentumDirection();
  auto totlength = step->GetStepLength();
  auto totdepo = step->GetTotalEnergyDeposit();
  auto density = step->GetTrack()->GetMaterial()->GetDensity()/(g/cm3);

  std::vector< std::pair<G4ThreeVector,G4double> > vecpoint;
  vecpoint.clear();

  if ((prepoint[0]>=coormin[0] and prepoint[0]<=coormax[0]) and (prepoint[1]>=coormin[1] and prepoint[1]<=coormax[1]) and (prepoint[2]>=coormin[2] and prepoint[2]<=coormax[2])){
    std::pair<G4ThreeVector,G4double> ori=std::make_pair(prepoint,0.);
    vecpoint.push_back(ori);
  }

  G4double tempxyz,tempstepxyz;
  G4ThreeVector temppoint;
  std::pair<G4ThreeVector,G4double> temppair;

  // x axis
  if (momentumdirection[0]>0) {
    for (G4int i =std::max(0,G4int(1+(prepoint[0]-coormin[0])/vecwidth[0]));i<=std::min(G4int(nbins[0]),G4int((postpoint[0]-coormin[0])/vecwidth[0]));i++){
      tempxyz=coormin[0]+vecwidth[0]*i;
      tempstepxyz=(tempxyz-prepoint[0])/momentumdirection[0];
      temppoint= G4ThreeVector(tempxyz,prepoint[1]+tempstepxyz*momentumdirection[1],prepoint[2]+tempstepxyz*momentumdirection[2]);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//        G4cout<<"in loop x: "<<i<<G4endl;
	//        G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }else if(momentumdirection[0]<0) {
    for (G4int i =std::min(G4int(nbins[0]),G4int((prepoint[0]-coormin[0])/vecwidth[0]));i>=std::max(0,G4int((postpoint[0]-coormin[0])/vecwidth[0]+1));i--){
      tempxyz=coormin[0]+vecwidth[0]*i;
      tempstepxyz=(tempxyz-prepoint[0])/momentumdirection[0];
      temppoint= G4ThreeVector(tempxyz,prepoint[1]+tempstepxyz*momentumdirection[1],prepoint[2]+tempstepxyz*momentumdirection[2]);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//G4cout<<"in loop x: "<<i<<G4endl;
	//G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }

  // y axis
  if (momentumdirection[1]>0) {
    for (G4int i =std::max(0,G4int(1+(prepoint[1]-coormin[1])/vecwidth[1]));i<=std::min(G4int(nbins[1]),G4int((postpoint[1]-coormin[1])/vecwidth[1]));i++){
      tempxyz=coormin[1]+vecwidth[1]*i;
      tempstepxyz=(tempxyz-prepoint[1])/momentumdirection[1];
      temppoint= G4ThreeVector(prepoint[0]+tempstepxyz*momentumdirection[0],tempxyz,prepoint[2]+tempstepxyz*momentumdirection[2]);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//      G4cout<<"in loop y: "<<i<<G4endl;
	//      G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }else if(momentumdirection[1]<0) {
    for (G4int i =std::min(G4int(nbins[1]),G4int((prepoint[1]-coormin[1])/vecwidth[1]));i>=std::max(0,G4int((postpoint[1]-coormin[1])/vecwidth[1]+1));i--){
      tempxyz=coormin[1]+vecwidth[1]*i;
      tempstepxyz=(tempxyz-prepoint[1])/momentumdirection[1];
      temppoint= G4ThreeVector(prepoint[0]+tempstepxyz*momentumdirection[0],tempxyz,prepoint[2]+tempstepxyz*momentumdirection[2]);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//      G4cout<<"in loop y: "<<i<<G4endl;
	//      G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }

  // z axis
  if (momentumdirection[2]>0) {
    for (G4int i =std::max(0,G4int(1+(prepoint[2]-coormin[2])/vecwidth[2]));i<=std::min(G4int(nbins[2]),G4int((postpoint[2]-coormin[2])/vecwidth[2]));i++){
      //      G4cout<<i<<" "<<std::max(0,G4int(1+(prepoint[2]-coormin[2])/vecwidth[2]))<<" "<<std::min(G4int(nbins[2]),G4int((postpoint[2]-coormin[2])/vecwidth[2]))<<G4endl;
      tempxyz=coormin[2]+vecwidth[2]*i;
      tempstepxyz=(tempxyz-prepoint[2])/momentumdirection[2];
      temppoint= G4ThreeVector(prepoint[0]+tempstepxyz*momentumdirection[0],prepoint[1]+tempstepxyz*momentumdirection[1],tempxyz);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//      G4cout<<"in loop z: "<<i<<G4endl;
	//      G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }else if(momentumdirection[2]<0) {
    for (G4int i =std::min(G4int(nbins[2]),G4int((prepoint[2]-coormin[2])/vecwidth[2]));i>=std::max(0,G4int((postpoint[2]-coormin[2])/vecwidth[2]+1));i--){
      //      G4cout<<i<<" "<<std::min(G4int(nbins[2]),G4int((prepoint[2]-coormin[2])/vecwidth[2]))<<" "<<std::max(0,G4int((postpoint[2]-coormin[2])/vecwidth[2]+1))<<G4endl;
      tempxyz=coormin[2]+vecwidth[2]*i;
      tempstepxyz=(tempxyz-prepoint[2])/momentumdirection[2];
      temppoint= G4ThreeVector(prepoint[0]+tempstepxyz*momentumdirection[0],prepoint[1]+tempstepxyz*momentumdirection[1],tempxyz);
      if ((temppoint[0]>=coormin[0] and temppoint[0]<=coormax[0]) and (temppoint[1]>=coormin[1] and temppoint[1]<=coormax[1]) and (temppoint[2]>=coormin[2] and temppoint[2]<=coormax[2])){
	temppair=std::make_pair(temppoint,(temppoint-prepoint).mag());
	vecpoint.push_back(temppair);
	//      G4cout<<"in loop z: "<<i<<G4endl;
	//      G4cout<<"temppoint: "<<temppoint<<G4endl;
      }
    }
  }
  if ((postpoint[0]>=coormin[0] and postpoint[0]<=coormax[0]) and (postpoint[1]>=coormin[1] and postpoint[1]<=coormax[1]) and (postpoint[2]>=coormin[2] and postpoint[2]<=coormax[2])){
    std::pair<G4ThreeVector,G4double> terminal=std::make_pair(postpoint,step->GetStepLength());
    vecpoint.push_back(terminal);

  }

  if (vecpoint.size()==0){return;}
  if (vecpoint.size()==1){
    //    G4cout<<"only one point in vecpoint......"<<G4endl;
    //    G4cout<<prepoint[0]<<" "<<prepoint[1]<<" "<<prepoint[2]<<G4endl;
    //    G4cout<<postpoint[0]<<" "<<postpoint[1]<<" "<<postpoint[2]<<G4endl;
    //    G4cout<<vecpoint[0].first[0]<<" "<<vecpoint[0].first[1]<<" "<<vecpoint[0].first[2]<<G4endl;
    vecpoint.push_back(vecpoint[0]);
  }
  sort(vecpoint.begin(),vecpoint.end(),cmppair);

  //  Run* run = static_cast<Run*>(
  //             G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  G4double ratio;

  for(std::vector< std::pair<G4ThreeVector,G4double> >::iterator iter=vecpoint.begin(); iter!=vecpoint.end()-1;iter++){
    if (totlength<=0){
      ratio=1.;
    }else{
      ratio=abs(((*iter).second-(*(iter+1)).second))/totlength;
    }
    //    G4cout<<(*iter).first[0]<<" "<<(*iter).first[1]<<" "<<(*iter).first[2]<<G4endl;
    //    G4cout<<(*(iter+1)).first[0]<<" "<<(*(iter+1)).first[1]<<" "<<(*(iter+1)).first[2]<<G4endl;
    temppoint = ((*iter).first+(*(iter+1)).first)/2.;
    G4int ijkbin[3];
    for (G4int i=0;i<3;i++){
      ijkbin[i] = G4int((temppoint[i] - coormin[i])/vecwidth[i]);
    }
    //    G4cout<<"depo: "<<std::setprecision(8)<<ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]<<" "<<ratio*totdepo<<G4endl;
    //    if (ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]>nbins[0]*nbins[1]*nbins[2] or ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]<0){
    if (ijkbin[0]<0 or ijkbin[0]>nbins[0] or ijkbin[1]<0 or ijkbin[1]>nbins[1] or ijkbin[2]<0 or ijkbin[2]>nbins[2]){
      G4cout<<"out of grids....."<<G4endl;
      G4cout<<"coor: "<<temppoint[0]<<" "<<temppoint[1]<<" "<<temppoint[2]<<G4endl;
      G4cout<<"      "<<ijkbin[0]<<" "<<ijkbin[1]<<" "<<ijkbin[2]<<G4endl;
      G4cout<<"vecwidth: "<<vecwidth[0]<<" "<<vecwidth[1]<<" "<<vecwidth[2]<<G4endl;
      G4cout<<"nbins: "<<nbins[0]<<" "<<nbins[1]<<" "<<nbins[2]<<G4endl;
      G4cout<<"coormin: "<<coormin[0]<<" "<<coormin[1]<<" "<<coormin[2]<<G4endl;
      G4cout<<"coormax: "<<coormax[0]<<" "<<coormax[1]<<" "<<coormax[2]<<G4endl;
      G4cout<<"test: "<<(*iter).first<<" "<<(*iter).second<<G4endl;
      G4cout<<"test: "<<(*(iter+1)).first<<" "<<(*iter).second<<G4endl;
    }else{
      G4double temptracklength = abs((*iter).second-(*(iter+1)).second)/cm;
      G4double kinenergy = step->GetPostStepPoint()->GetKineticEnergy()/GeV;
      fVector[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=ratio*(totdepo/GeV)/density;
      fnneu[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;
      if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 2112 ){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
	Calcudose(fdoseeqright, nbinscoeffdoseeqneu, coeffneu, ijkbin, temptracklength, kinenergy);
	Calcudose(f1mevnfluen,  nbinscoeff1mevneueqfluenceneu, coeff1mevneueqfluenceneu, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 2212 ){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
	Calcudose(fdoseeqright, nbinscoeffdoseeqproton, coeffproton, ijkbin, temptracklength, kinenergy);
	Calcudose(f1mevnfluen,  nbinscoeff1mevneueqfluenceproton, coeff1mevneueqfluenceproton, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 1000020030 or (step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 1000020040 ){
	// He3                                                                   He4
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
	Calcudose(fdoseeqright, nbinscoeffdoseeqhe, coeffhe, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 22 ){
	Calcudose(fdoseeqright, nbinscoeffdoseeqphoton, coeffphoton, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 11 ){
	Calcudose(fdoseeqright, nbinscoeffdoseeqele, coeffele, ijkbin, temptracklength, kinenergy);
	Calcudose(f1mevnfluen,  nbinscoeff1mevneueqfluenceele, coeff1mevneueqfluenceele, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == -11 ){
	Calcudose(fdoseeqright, nbinscoeffdoseeqpos, coeffpos, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 13 ){
	Calcudose(fdoseeqright, nbinscoeffdoseeqmum, coeffmum, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == -13 ){
	Calcudose(fdoseeqright, nbinscoeffdoseeqmup, coeffmup, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 211 ){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
	Calcudose(fdoseeqright, nbinscoeffdoseeqpip, coeffpip, ijkbin, temptracklength, kinenergy);
	Calcudose(f1mevnfluen,  nbinscoeff1mevneueqfluencepion, coeff1mevneueqfluencepion, ijkbin, temptracklength, kinenergy);
      }else if ((step->GetTrack()->GetDefinition()->GetPDGEncoding()) == -211 ){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
	Calcudose(fdoseeqright, nbinscoeffdoseeqpim, coeffpim, ijkbin, temptracklength, kinenergy);
	Calcudose(f1mevnfluen,  nbinscoeff1mevneueqfluencepion, coeff1mevneueqfluencepion, ijkbin, temptracklength, kinenergy);
      }else if ((abs(step->GetTrack()->GetDefinition()->GetPDGEncoding()) >= 310) and (abs(step->GetTrack()->GetDefinition()->GetPDGEncoding()) <= 321)){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
      }else if (abs(step->GetTrack()->GetDefinition()->GetPDGEncoding()) == 130){
	if(kinenergy>hehadcut) {fhehadfluen[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength;}
      }
    }
  }
}
////....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void ExampleAnaDoseElemTool::Writefile(std::string prefixion)
{
  G4cout << "write file paras. " <<nbins<<coormin<<coormax<<regionhist<< G4endl;
  std::ofstream file(prefixion+"edep.dat");
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    file<<fVector[i]<<std::endl;
  }
  file.close();
  std::ofstream file2(prefixion+"nneu.dat");
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    file2<<fnneu[i]<<std::endl;
  }
  file2.close();
  //  std::ofstream file3("det1.dat");
  //  for (G4int i = 0;i<regionhist[0];i++){
  //    file3<<fdet1[i]<<std::endl;
  //    //      std::cout<<fdet1[i]<<std::endl;
  //  }
  //  file3.close();
  //  std::ofstream file4("air.dat");
  //  for (G4int i = 0;i<regionhist[0];i++){
  //    file4<<fair[i]<<std::endl;
  //    //      std::cout<<fair[i]<<std::endl;
  //  }
  //  file4.close();
  std::ofstream file5(prefixion+"doseeq.dat");
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    file5<<fdoseeqright[i]<<std::endl;
  }
  file5.close();
  std::ofstream file6(prefixion+"1mevneqfluence.dat");
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    file6<<f1mevnfluen[i]<<std::endl;
  }
  file6.close();
  std::ofstream file7(prefixion+"hehadfluence.dat");
  for (G4int i = 0;i<nbins[0]*nbins[1]*nbins[2];i++){
    file7<<fhehadfluen[i]<<std::endl;
  }
  file7.close();
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ExampleAnaDoseElemTool::Readcoffe(std::string filepath, std::string filename, const G4int constnbins, G4double coeffexample[][2]){
  //  std::string filename="n_GeV_icru95joint.txt";
  m_inputFile_doseeqneu.open(filepath+filename);
  if (!m_inputFile_doseeqneu){
    std::cout << "SteppingAction: PROBLEMS OPENING FILE " <<filename<< std::endl;
    exit(0);
  }
  for (int i = 0; i < constnbins; i++){
    m_inputFile_doseeqneu >> coeffexample[i][0]>> coeffexample[i][1];
    if( coeffexample[i][0]<=0. or coeffexample[i][1]<=0.){
      std::cout<<"There is Zero value: "<<coeffexample[i][0]<<" "<<coeffexample[i][1]<<std::endl;
    }
  }
  std::cout<<"coff: "<<filename<<"  "<<constnbins<<std::endl;
  //  for (int i = 0; i < constnbins; i++){
  //    std::cout<<coeffexample[i][0]<<" "<<coeffexample[i][1]<<std::endl;
  //  }
  m_inputFile_doseeqneu.close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void ExampleAnaDoseElemTool::Calcudose(std::vector<G4double>& fexample, const G4int constnbins, const G4double coeffexample[][2], const G4int ijkbin[], const G4double inputtracklength, const G4double inputkinenergy){
  if(inputkinenergy >= coeffexample[constnbins-1][0]){
    fexample[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=inputtracklength* coeffexample[constnbins-1][1];
    //    fdoseeqright[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength* coeffexample[constnbins-1][1];
    //    fdoseeqleft[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]] +=temptracklength* coeffexample[constnbins-1][1];
  }else if(inputkinenergy <= coeffexample[0][0]){
    fexample[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=inputtracklength* coeffexample[0][1];
    //    fdoseeqright[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength* coeffexample[0][1];
    //    fdoseeqleft[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]] +=temptracklength* coeffexample[0][1];
  }else{
    for(G4int jj=1;jj<nbinscoeffdoseeqneu;jj++){
      if( inputkinenergy<coeffexample[jj][0]){
	fexample[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=inputtracklength* coeffexample[jj][1];
	//      fdoseeqright[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]]+=temptracklength* coeffexample[jj][1];
	//      fdoseeqleft[ijkbin[0]*nbins[1]*nbins[2]+ijkbin[1]*nbins[2]+ijkbin[2]] +=temptracklength* coeffexample[jj-1][1];
	break;
      }
    }
  }
}
