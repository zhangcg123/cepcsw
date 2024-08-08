#ifndef XTALECALENERGYCORRECTIONSvc_C
#define XTALECALENERGYCORRECTIONSvc_C

#include "CrystalEcalEnergyCorrectionSvc.h"

DECLARE_COMPONENT(CrystalEcalEnergyCorrectionSvc)

double CrystalEcalEnergyCorrectionSvc::thetaCorrectionAngleStart[7] = {94.5, 105, 115, 123.5, 130.5, 136.5, 141};
double CrystalEcalEnergyCorrectionSvc::thetaCorrectionAngleEnd[7] = {97.0, 107.5, 117.5, 126, 133, 139.0, 143.5};
double CrystalEcalEnergyCorrectionSvc::phiCorrectionAngleStart[2] = {4, 15.5};
double CrystalEcalEnergyCorrectionSvc::phiCorrectionAngleEnd[2] = {7, 18.5};
double CrystalEcalEnergyCorrectionSvc::mpvCorrectionFactor[5] = {-1524.86, -2098.74, 0.01, 191.79, 0.98};
int CrystalEcalEnergyCorrectionSvc::angleBinNumber = 100;
double CrystalEcalEnergyCorrectionSvc::angleRangePhi = 3.0;
double CrystalEcalEnergyCorrectionSvc::angleRangeTheta = 2.5;
int CrystalEcalEnergyCorrectionSvc::moduleNumberPhi = 32;

StatusCode CrystalEcalEnergyCorrectionSvc::initialize() {
    
    // read correction file
    std::string m_correctionFile = _correctionFile;
    file = TFile::Open(m_correctionFile.c_str());

    // phi
    treePhi = (TTree*)(file->Get("scalePhi")); 
    treePhi->SetBranchAddress("phi", &_phiAngle);
    treePhi->SetBranchAddress("scale", &_phiScale); 
    for(int i = 0; i < treePhi->GetEntries(); i++) {
        treePhi->GetEntry(i);
        phiAngle.push_back(_phiAngle);
        phiScale.push_back(_phiScale);
    }

    //theta
    treeTheta = (TTree*)(file->Get("scaleTheta"));
    treeTheta->SetBranchAddress("theta", &_thetaAngle);
    treeTheta->SetBranchAddress("scale", &_thetaScale);
    for(int i = 0; i < treeTheta->GetEntries(); i++) {
        treeTheta->GetEntry(i);
        thetaAngle.push_back(_thetaAngle);
        thetaScale.push_back(_thetaScale);
    }

    StatusCode sc = Service::initialize();
    return sc;
}

StatusCode CrystalEcalEnergyCorrectionSvc::finalize() {
    
    file->Close();

    phiAngle.clear();
    phiScale.clear();
    thetaAngle.clear();
    thetaScale.clear();
    
    StatusCode sc = Service::finalize();
    return sc;
}

double CrystalEcalEnergyCorrectionSvc::energyCorrection(double energy, double phi, double theta) { // energy: GeV, phi: 0-360, theta: 0-180

    //std::cout<<"Before correction: "<<energy<<" phi "<<phi<<" theta "<<theta<<std::endl;

    // #####  mpv deviation #####
    double energyCor = 0;
    energyCor = energy / (mpvCorrectionFactor[3]/(mpvCorrectionFactor[0] + mpvCorrectionFactor[1]*energy + mpvCorrectionFactor[2]*energy*energy) + mpvCorrectionFactor[4]);
    //std::cout<<"After MPV correction: "<<energyCor<<std::endl;

    // #####  energy leakage in cracks #####
    
    double tmpPhi = fmod(phi, 360./(moduleNumberPhi/2));

    auto itPhiStart = std::lower_bound(std::begin(phiCorrectionAngleStart), std::end(phiCorrectionAngleStart), tmpPhi);
    auto itPhiEnd = std::upper_bound(std::begin(phiCorrectionAngleEnd), std::end(phiCorrectionAngleEnd), tmpPhi);

    int iPhiStart = std::distance(std::begin(phiCorrectionAngleStart), itPhiStart) - 1;
    int jPhiEnd = std::distance(std::begin(phiCorrectionAngleEnd), itPhiEnd);

    if(iPhiStart == jPhiEnd) {
        
        int binNumber = jPhiEnd*angleBinNumber + abs((tmpPhi-phiCorrectionAngleStart[iPhiStart])/(angleRangePhi/angleBinNumber));
        //std::cout<<"   phi correction factor: "<<phiScale.at(binNumber)<<std::endl;
        energyCor = energyCor/phiScale.at(binNumber);
    }

    //std::cout<<"After phi correction: "<<energyCor<<std::endl;
    
    double tmpTheta = theta;
    if(tmpTheta<90) tmpTheta = 90 + (90-tmpTheta);

    auto itThetaStart = std::lower_bound(std::begin(thetaCorrectionAngleStart), std::end(thetaCorrectionAngleStart), tmpTheta);
    auto itThetaEnd = std::upper_bound(std::begin(thetaCorrectionAngleEnd), std::end(thetaCorrectionAngleEnd), tmpTheta);

    int iThetaStart = std::distance(std::begin(thetaCorrectionAngleStart), itThetaStart) - 1;
    int jThetaEnd = std::distance(std::begin(thetaCorrectionAngleEnd), itThetaEnd);

    if(iThetaStart == jThetaEnd) {
        
        int binNumber = jThetaEnd*angleBinNumber + abs((tmpTheta-thetaCorrectionAngleStart[iThetaStart])/(angleRangeTheta/angleBinNumber));
        //std::cout<<"   theta correction factor: "<<thetaScale.at(binNumber)<<std::endl;
        energyCor = energyCor/thetaScale.at(binNumber);
    }

    //std::cout<<"After theta correction: "<<energyCor<<std::endl;
    
    //msg() << "corection done!" << endmsg;
    return energyCor;
}
#endif
