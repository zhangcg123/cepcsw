#ifndef IXTALECALSvc_h
#define IXTALECALSvc_h

#include "GaudiKernel/IService.h"

class ICrystalEcalSvc: virtual public IInterface {
public:
    DeclareInterfaceID(ICrystalEcalSvc, 0, 1); // major/minor version
    virtual ~ICrystalEcalSvc() = default;

    virtual double energyCorrection(double energy, double phi, double theta) = 0;

    // virtual void ClearSystem() = 0;

private:

};

#endif
