#include "SimplePIDSvc.h"
#include "DataHelper/HelixClass.h"
#include "GearSvc/IGearSvc.h"
#include "gear/BField.h"
#include "TFile.h"
#include "TH2D.h"
#include "TRandom.h"
#include <iostream>
#include <cmath>

DECLARE_COMPONENT(SimplePIDSvc)

using namespace edm4hep;

SimplePIDSvc::SimplePIDSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc)
{
    m_rootFile = nullptr;
}

SimplePIDSvc::~SimplePIDSvc()
{
    if (m_rootFile != nullptr) delete m_rootFile;
}

double SimplePIDSvc::getDndx(double mean, double sigma) {
    return gRandom->Gaus(mean, sigma);
}

double SimplePIDSvc::getDndxMean(double bg, double cos)
{
    return interpolate(m_dndxMean, bg, cos);
}

double SimplePIDSvc::getDndxSigma(double bg, double cos, double len)
{
    return interpolate(m_dndxSigma, bg, cos)/sqrt(len*0.1); // len in mm, need to convert to cm
}

double SimplePIDSvc::getChi2(double dndx_meas, double dndx_exp, double dndx_sigma) {
    double chi = (dndx_meas - dndx_exp)/dndx_sigma;
    return chi*chi;
}

void SimplePIDSvc::getTrackPars(const edm4hep::Vector3d& first_hit_pos, const edm4hep::Vector3d& last_hit_pos, const edm4hep::Track& trk, int location,
                                double& length, double& p, double& costheta) 
{
    TrackState trk_par;
    for (auto it = trk.trackStates_begin(); it != trk.trackStates_end(); it++) {
        if (it->location == location) {
            trk_par = *it;
            break;
        }
    }
	auto _gear = service<IGearSvc>("GearSvc");
	double bfield = _gear->getGearMgr()->getBField().at(gear::Vector3D(0.,0.0,0.)).z();
    HelixClass hc = HelixClass();
    hc.Initialize_Canonical(trk_par.phi, trk_par.D0, trk_par.Z0, trk_par.omega, trk_par.tanLambda, bfield);
    double R = hc.getRadius();
    double phi1 = atan2(first_hit_pos[1] - hc.getYC(), first_hit_pos[0] - hc.getXC());
    double phi2 = atan2(last_hit_pos[1] - hc.getYC(), last_hit_pos[0] - hc.getXC());
    double z1 = first_hit_pos[2];
    double z2 = last_hit_pos[2];


    // std::cout << "xc, yc = " << hc.getXC() << ", " << hc.getYC() << std::endl;
    // std::cout << "r = " << R << std::endl;
    // std::cout << "x1, y1 = " << first_hit_pos[0] << ", " << first_hit_pos[1] << std::endl;
    // std::cout << "x2, y2 = " << last_hit_pos[0] << ", " << last_hit_pos[1] << std::endl;
    // std::cout << "phi1, phi2 = " << phi1 << ", " << phi2 << std::endl;
    // std::cout << "tanLambda = " << trk_par.tanLambda << std::endl;
    // std::cout << "z1, z2 = " << z1 << ", " << z2 << std::endl;
    // const float* p3 = hc.getMomentum();
    // std::cout << "momentum = " << sqrt(p3[0]*p3[0] + p3[1]*p3[1] + p3[2]*p3[2]) << std::endl;
    // std::cout << "charge = " << trk_par.omega << std::endl;

    // if (fabs(z2 - z1) > fabs(2*M_PI*R/cos(atan(trk_par.tanLambda)))) { // length is larger than one cycle
    if (fabs(trk_par.tanLambda) > 0.1) { // if the track is not vertical, use z
        length = fabs((z1 - z2)/sin(atan(trk_par.tanLambda)));
    }
    else {
        int outward_direction = int(fabs(z1) < fabs(z2));
        int positive_charge = int(trk_par.omega > 0.);
        if (outward_direction * positive_charge > 0) {
            length = phi1 > phi2 ? (phi1 - phi2)*R/cos(atan(trk_par.tanLambda)) : (phi1 - phi2 + 2*M_PI)*R/cos(atan(trk_par.tanLambda));
        }
        else {
            length = phi1 < phi2 ? (phi2 - phi1)*R/cos(atan(trk_par.tanLambda)) : (phi2 - phi1 + 2*M_PI)*R/cos(atan(trk_par.tanLambda));
        }
    }
    // std::cout << "length (rphi): " << fabs((phi1 - phi2)*R/cos(atan(trk_par.tanLambda))) << std::endl;
    // std::cout << "length (z): " << fabs((z1 - z2)/sin(atan(trk_par.tanLambda))) << std::endl << std::endl;
    // std::cout << "length: " << length << std::endl;

    double pt = hc.getPXY();
    double pz = pt * hc.getTanLambda();
    p = sqrt(pt*pt + pz*pz);
    costheta = cos(M_PI/2 - atan(hc.getTanLambda()));
}

double SimplePIDSvc::getBetaGamma(double p, int pid_type) { // p: GeV/c
    double mass;
    switch (pid_type)
    {
    case 0:
        mass = 0.511;
        break;
    case 1:
        mass = 105.658;
        break;
    case 2:
        mass = 139.570;
        break;
    case 3:
        mass = 493.677;
        break;
    case 4:
        mass = 938.272;
        break;
    
    default:
        mass = 105.658; // default is muon
        break;
    }
    return p/mass*1000.;
}

int SimplePIDSvc::getParticleType(int pdgid) {
    int type;
    switch (abs(pdgid))
    {
    case 11:
        type = 0;
        break;
    case 13:
        type = 1;
        break;
    case 211:
        type = 2;
        break;
    case 321:
        type = 3;
        break;
    case 2212:
        type = 4;
        break;
    
    default:
        type = -1;
        break;
    }
    return type;
}

StatusCode SimplePIDSvc::initialize()
{
    m_rootFile = new TFile(m_parFile.value().c_str());
    m_dndxMean = (TH2D*)m_rootFile->Get("dndx_mean");
    m_dndxSigma = (TH2D*)m_rootFile->Get("dndx_sigma");
    return StatusCode::SUCCESS;
}

StatusCode SimplePIDSvc::finalize()
{
    if (m_rootFile != nullptr) delete m_rootFile;
    return StatusCode::SUCCESS;
}

double SimplePIDSvc::interpolate(TH2D* h, double x, double y) {
    int nx = h->GetNbinsX();
    int ny = h->GetNbinsY();
    double xmax = h->GetXaxis()->GetBinCenter(nx);
    double xmin = h->GetXaxis()->GetBinCenter(1);
    double ymax = h->GetYaxis()->GetBinCenter(ny);
    double ymin = h->GetYaxis()->GetBinCenter(1);
    double dlogx = (log10(xmax) - log10(xmin)) * 0.01;
    double dy = (ymax - ymin) * 0.01;

    if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
        return h->Interpolate(x, y);
    }

    int xloc, yloc;
    if (x > xmax) xloc = 1;
    else if (x < xmin) xloc = -1;
    else xloc = 0;
    if (y > ymax) yloc = 1;
    else if (y < ymin) yloc = -1;
    else yloc = 0;

    double x1, x2, y1, y2, z1, z2;
    if (xloc == 1) {
        x1 = pow(10, log10(xmax)-dlogx);
        x2 = xmax;
    }
    else if (xloc == -1) {
        x1 = xmin;
        x2 = pow(10, log10(xmin) + dlogx);
    }
    if (yloc == 1) {
        y1 = ymax - dy;
        y2 = ymax;
    }
    else if (yloc == -1) {
        y1 = ymin;
        y2 = ymin + dy;
    }

    if (xloc != 0 && yloc == 0) {
        z1 = h->Interpolate(x1, y);
        z2 = h->Interpolate(x2, y);
        return linear_interpolate(x1, x2, z1, z2, x);
    }
    else if (xloc == 0 && yloc != 0) {
        z1 = h->Interpolate(x, y1);
        z2 = h->Interpolate(x, y2);
        return linear_interpolate(y1, y2, z1, z2, y);
    }
    else if (xloc != 0 && yloc != 0) {
        double y_boundary = yloc > 0 ? ymax : ymin;
        z1 = h->Interpolate(x1, y_boundary);
        z2 = h->Interpolate(x2, y_boundary);
        double z_tmp = linear_interpolate(x1, x2, z1, z2, x);

        double x_boundary = xloc > 0 ? xmax : xmin;
        z1 = h->Interpolate(x_boundary, y1);
        z2 = h->Interpolate(x_boundary, y2);
        double z_tmp2 = linear_interpolate(y1, y2, z1, z2, y);

        return z_tmp + z_tmp2 - h->Interpolate(x_boundary, y_boundary);
    }
    else {
        return 0.;
    }
}

double SimplePIDSvc::linear_interpolate(double x1, double x2, double y1, double y2, double x) {
    return (y2-y1)/(x2-x1)*(x-x1) + y1;
}
