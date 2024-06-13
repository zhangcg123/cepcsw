#ifndef I_SimplePID_SVC_H
#define I_SimplePID_SVC_H

#include "GaudiKernel/IService.h"
#include "edm4hep/Vector3d.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/Track.h"
#include "edm4hep/Vector3d.h"

/**
 * @class ISimplePIDSvc
 * @brief Interface for Simple PID
 * @author Guang Zhao (zhaog@ihep.ac.cn)
*/

class ISimplePIDSvc: virtual public IService {
public:
    DeclareInterfaceID(ISimplePIDSvc, 0, 1); // major/minor version
    
    virtual ~ISimplePIDSvc() = default;

    virtual double getDndx(double mean, double sigma) = 0;
    virtual double getDndxMean(double bg, double cos) = 0;
    virtual double getDndxSigma(double bg, double cos, double len) = 0;
    virtual double getChi2(double dndx_meas, double dndx_exp, double dndx_sigma) = 0;
    virtual void getTrackPars(const edm4hep::Vector3d& hit1, const edm4hep::Vector3d& hit2, const edm4hep::Track& trk, int location,
                              double& len, double& p, double& cos) = 0;
    virtual double getBetaGamma(double p, int pid_type) = 0; // e, mu, pi, K, p: 0, 1, 2, 3, 4
    virtual int getParticleType(int pdgid) = 0;
};


#endif
