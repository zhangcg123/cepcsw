#ifndef _CRD_CALOCRYSTALSHORT_
#define _CRD_CALOCRYSTALSHORT_

#include <DD4hep/Objects.h>
#include "TVector3.h"

class CaloCrystalShort
{
public:
    CaloCrystalShort(unsigned long long _cellID, int _system, int _module, int _stave, int _layer, int _phi_x, int _z_y, TVector3 _pos, double _Q, double _T)
            : cellID(_cellID), system(_system), module(_module), stave(_stave), layer(_layer), phi_x(_phi_x), z_y(_z_y), position(_pos), Q(_Q), T(_T) {};
    /*
    CaloCrystalShort(unsigned long long _cellID, int _system, int _module, int _stave, int _layer, int _x, int _y, TVector3 _pos, double _Q, double _T)
            : cellID(_cellID), system(_system), module(_module), stave(_stave), layer(_layer), x(_x), y(_y), position(_pos), Q(_Q), T(_T) {};
    */

    CaloCrystalShort() {};

    inline bool operator==(const CaloCrystalShort& x) const
    {
        return ((cellID == x.cellID) && getEnergy() == x.getEnergy());
    }

    unsigned long long getcellID() const { return cellID; }

    int getSystem() const { return system; }

    int getModule() const { return module; }

    int getStave() const { return stave; }

    int getLayer() const { return layer; }

    int getPhiX() const { return phi_x; }

    int getZY() const { return z_y; }

    double getQ() const { return Q; }

    double getT() const { return T; }

    TVector3 getPosition() const { return position; }

    double getEnergy() const { return Q; }

    void setcellID(unsigned long long _cellid) { cellID = _cellid; }

    void setcellID(int _system, int _module, int _stave, int _layer, int _phi_x, int _z_y)
    {
        system = _system;
        module = _module;
        stave = _stave;
        layer = _layer;
        phi_x = _phi_x;
        z_y = _z_y;
    }

    /*
    void setcellID(int _system, int _module, int _stave, int _layer, int _x, int _y)
    {
        system = _system;
        module = _module;
        stave = _stave;
        layer = _layer;
        x = _x;
        y = _y;
    }
     */

    void setPosition(TVector3 posv3) { position.SetXYZ(posv3.x(), posv3.y(), posv3.z()); }

    void setQ(double _q) { Q = _q; }

    void setT(double _t) { T = _t; }

private:
    unsigned long long cellID;
    int system;
    int module;
    int stave;
    int layer;
    int phi_x;
    int z_y;
    TVector3 position;
    double Q;    // Charge in readout
    double T;    // Time in readout;
};

#endif
