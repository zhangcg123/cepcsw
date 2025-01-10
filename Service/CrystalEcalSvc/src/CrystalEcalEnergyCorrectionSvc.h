#ifndef XTALECALENERGYCORRECTIONSvc_h
#define XTALECALENERGYCORRECTIONSvc_h

#include <GaudiKernel/Service.h>
#include "CrystalEcalSvc/ICrystalEcalSvc.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include <vector>
#include <string>
#include <cfloat>
#include <cmath>

class CrystalEcalEnergyCorrectionSvc: public extends<Service, ICrystalEcalSvc> {
public:
    using extends::extends;

    StatusCode initialize() override;
    StatusCode finalize() override;

    double energyCorrection(double energy, double phi, double theta) override;
    std::string doubleToString(double number);

    static double thetaCorrectionAngleStart[7];
    static double thetaCorrectionAngleEnd[7];
    static double phiCorrectionAngleStart[2];
    static double phiCorrectionAngleEnd[2];
    static double mpvCorrectionFactor[5];
    static int angleBinNumber;
    static double angleRangePhi;
    static double angleRangeTheta;
    static int moduleNumberPhi;

private:
    mutable Gaudi::Property<std::string> _correctionFile{this, "CorrectionFile", "", "position of energy correction file"};

    TFile* file;
    TTree* barrelPhiCorrection;
    TTree* barrelThetaCorrection; 
    TTree* endcapCorrection; 
    double _barrelPhiAngle, _barrelPhiScale, _barrelThetaAngle, _barrelThetaScale;
    std::vector<double> barrelPhiAngle, barrelPhiScale, barrelThetaAngle, barrelThetaScale; 
    double _endcapTheta, _endcapPhi, _endcapScale;
    // std::vector<double> endcapTheta, endcapPhi, endcapScale;

    std::map<std::tuple<std::string, std::string>, double> endcapCorrectionMap;
    // TTree* treePhi;
    // TTree* treeTheta; 
    // double _phiAngle, _phiScale, _thetaAngle, _thetaScale;
    // std::vector<double> phiAngle, phiScale, thetaAngle, thetaScale; 

};

#endif
