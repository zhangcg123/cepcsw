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
    mutable Gaudi::Property<std::string> _correctionFile{this, "CorrectionFile", "/cefs/higgs/songwz/summer24/CEPCSW_301/CEPCSW/workArea/scale.root", "position of energy correction file"};
    
    TFile* file;
    TTree* treePhi;
    TTree* treeTheta; 
    double _phiAngle, _phiScale, _thetaAngle, _thetaScale;
    std::vector<double> phiAngle, phiScale, thetaAngle, thetaScale; 
      
};

#endif
