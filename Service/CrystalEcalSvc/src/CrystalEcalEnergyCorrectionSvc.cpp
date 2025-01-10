#ifndef XTALECALENERGYCORRECTIONSvc_C
#define XTALECALENERGYCORRECTIONSvc_C

#include "CrystalEcalEnergyCorrectionSvc.h"
#include <iostream>
#include <sstream>
#include <iomanip>

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
    if(m_correctionFile.empty()){ 
      error() << "CrystalEcalEnergyCorrectionSvc error: can not find correction map file! " << endmsg;
      return StatusCode::FAILURE;
    }
    file = TFile::Open(m_correctionFile.c_str());
    if(!file->IsOpen()){ 
      error() << "CrystalEcalEnergyCorrectionSvc error: can not find correction map file! " << endmsg;
      return StatusCode::FAILURE;
    }
    // phi
    barrelPhiCorrection = (TTree*)(file->Get("scalePhi")); 
    barrelPhiCorrection->SetBranchAddress("phi", &_barrelPhiAngle);
    barrelPhiCorrection->SetBranchAddress("scale", &_barrelPhiScale); 
    for(int i = 0; i < barrelPhiCorrection->GetEntries(); i++) {
        barrelPhiCorrection->GetEntry(i);
        //barrelPhiAngle.push_back(_barrelPhiAngle);
        barrelPhiScale.push_back(_barrelPhiScale);
    }

    //theta
    barrelThetaCorrection = (TTree*)(file->Get("scaleTheta"));
    barrelThetaCorrection->SetBranchAddress("theta", &_barrelThetaAngle);
    barrelThetaCorrection->SetBranchAddress("scale", &_barrelThetaScale);
    for(int i = 0; i < barrelThetaCorrection->GetEntries(); i++) {
        barrelThetaCorrection->GetEntry(i);
        //barrelThetaAngle.push_back(_barrelThetaAngle);
        barrelThetaScale.push_back(_barrelThetaScale);
    }

    // endcap
    endcapCorrection = (TTree*)(file->Get("EcalEndcapEnergyCorrection"));
    endcapCorrection->SetBranchAddress("theta", &_endcapTheta);
    endcapCorrection->SetBranchAddress("phi", &_endcapPhi);
    endcapCorrection->SetBranchAddress("mpv", &_endcapScale);
    for(int i = 0; i < endcapCorrection->GetEntries(); i++) {
        endcapCorrection->GetEntry(i);

        std::string stringTheta = doubleToString(_endcapTheta);
        //std::cout<<"theta: "<<stringTheta<<std::endl;
        std::string stringPhi = doubleToString(_endcapPhi);
        //std::cout<<"phi: "<<stringPhi<<std::endl;
        endcapCorrectionMap[std::make_tuple(stringTheta, stringPhi)] = _endcapScale;
    }

    

    StatusCode sc = Service::initialize();
    return sc;
}

StatusCode CrystalEcalEnergyCorrectionSvc::finalize() {
    
    file->Close();

    //barrelPhiAngle.clear();
    barrelPhiScale.clear();
    //barrelThetaAngle.clear();
    barrelThetaScale.clear();
    // endcapTheta.clear();
    // endcapPhi.clear();
    // endcapScale.clear();
    
    StatusCode sc = Service::finalize();
    return sc;
}

double CrystalEcalEnergyCorrectionSvc::energyCorrection(double energy, double phi, double theta) { // energy: GeV, phi: 0-360, theta: 0-180

    //std::cout<<"Before correction: "<<energy<<" phi "<<phi<<" theta "<<theta<<std::endl;
    double energyCor = 0;

    // #####  endcap correction #####
    if(theta <36 || theta > 144) {
        double tmpTheta = theta;
        if(tmpTheta>90) tmpTheta = 90 - (tmpTheta-90);
        double tmpPhi = phi;
        
        std::string stringTheta = doubleToString(tmpTheta);
        std::string stringPhi = doubleToString(tmpPhi);
        //std::cout<<"   endcap correction factor: "<<endcapCorrectionMap[std::make_tuple(stringTheta, stringPhi)]<<std::endl;
        energyCor = energy/endcapCorrectionMap[std::make_tuple(stringTheta, stringPhi)];
        return energyCor;
    }

    // #####  mpv deviation #####
    
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
        //std::cout<<"   phi correction factor: "<<barrelPhiScale.at(binNumber)<<std::endl;
        energyCor = energyCor/barrelPhiScale.at(binNumber);
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
        //std::cout<<"   theta correction factor: "<<barrelThetaScale.at(binNumber)<<std::endl;
        energyCor = energyCor/barrelThetaScale.at(binNumber);
    }

    //std::cout<<"After theta correction: "<<energyCor<<std::endl;
    
    //msg() << "corection done!" << endmsg;
    return energyCor;
}

std::string CrystalEcalEnergyCorrectionSvc::doubleToString(double number) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << number;
    return oss.str();
}

#endif
