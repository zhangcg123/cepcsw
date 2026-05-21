#ifndef SIMPLEPID_SVC_H
#define SIMPLEPID_SVC_H

#include "SimplePIDSvc/ISimplePIDSvc.h"
#include <GaudiKernel/Service.h>
#include "DD4hep/Detector.h"
#include "gear/GearMgr.h"
#include "edm4hep/Vector3d.h"

class TFile;
class TH2D;

/**
 * @class SimplePIDSvc
 * @brief Simple PID service
 * @author Guang Zhao (zhaog@ihep.ac.cn)
*/

class SimplePIDSvc : public extends<Service, ISimplePIDSvc>
{
    public:
        SimplePIDSvc(const std::string& name, ISvcLocator* svc);
        virtual ~SimplePIDSvc();

        double getDndx(double mean, double sigma) override;
        double getDndxMean(double bg, double cos) override;
        double getDndxSigma(double bg, double cos, double len) override;
        double getChi2(double dndx_meas, double dndx_exp, double dndx_sigma) override;
        void getTrackPars(const edm4hep::Vector3d& hit1, const edm4hep::Vector3d& hit2, const edm4hep::Track& trk, int location,
                          double& len, double& p, double& cos) override;
        double getBetaGamma(double p, int pid_type) override;
        int getParticleType(int pdgid) override;

        StatusCode initialize() override;
        StatusCode finalize() override;

    private:
        Gaudi::Property<std::string> m_parFile{this, "ParFile", "dNdx_TPC.root"};
        TFile* m_rootFile;
        TH2D* m_dndxMean;
        TH2D* m_dndxSigma;
        double interpolate(TH2D* h, double bg, double cos);
        double linear_interpolate(double x1, double x2, double y1, double y2, double x);
};

#endif
