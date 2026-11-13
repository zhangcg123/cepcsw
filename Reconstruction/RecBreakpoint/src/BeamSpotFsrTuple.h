#ifndef BP_BEAM_SPOT_FSR_TUPLE_H
#define BP_BEAM_SPOT_FSR_TUPLE_H
#include "BeamSpotFsrFitter.h"
#include "TTree.h"
#include <array>
#include <limits>
namespace breakpoint {
struct FsrFamilyTuple {
  static constexpr double missing=std::numeric_limits<double>::quiet_NaN();
  int status=0,mask=0,active=0,direction=0;
  double energy=missing,variance=missing,px=missing,py=missing,pz=missing;
  double primary=missing,neutral=missing,closure=missing,materialClosure=missing,nll2=missing;
  std::vector<double> electronIp,electronP4,jointIpEnergy,jointP4;
  std::string error;
  void book(TTree& tree,const std::string& prefix) {
    auto branch=[&](const char* suffix,auto* value){tree.Branch((prefix+suffix).c_str(),value);};
    branch("status",&status);branch("constraint_mask",&mask);branch("active_boundary_mask",&active);
    branch("direction_mode",&direction);
    branch("energy",&energy);branch("energy_variance",&variance);
    branch("px",&px);branch("py",&py);branch("pz",&pz);
    branch("primary_ecal_energy",&primary);branch("neutral_ecal_energy",&neutral);
    branch("nonlinear_closure",&closure);branch("material_closure",&materialClosure);branch("nll2",&nll2);
    branch("electron_ip_parameters",&electronIp);branch("electron_p4",&electronP4);
    branch("joint_ip_energy_covariance",&jointIpEnergy);branch("joint_p4_covariance",&jointP4);
    branch("error",&error);
  }
  void assignElectron(const edm4hep::TrackState& ip,int state=1) {
    auto& f=*this;f.status=state;
    f.electronIp={ip.D0,ip.phi,ip.omega,ip.Z0,ip.tanLambda};
  }
  void assign(const BeamSpotFsrResult& result,int actualMask,int state=2) {
    assignElectron(result.post,state);auto& f=*this;
    f.mask=actualMask;f.active=result.activeMask;f.direction=1;
    f.energy=result.energy;f.variance=result.variance;
    f.px=result.p4.photon[1];f.py=result.p4.photon[2];f.pz=result.p4.photon[3];
    f.primary=result.firstEcalPart;f.neutral=result.neutralPart;
    f.closure=result.nonlinearClosure;f.materialClosure=result.materialClosure;f.nll2=result.likelihood.nll2;
    f.electronP4.assign(result.p4.electron.begin(),result.p4.electron.end());
    f.jointIpEnergy=result.jointIpEnergyCovariance;
    f.jointP4.assign(result.p4.covariance.begin(),result.p4.covariance.end());
  }
};
struct BeamSpotFsrTuple {
  std::array<FsrFamilyTuple,4> families;
  void book(TTree& tree) {
    const char* prefixes[]={"free_loss_fsr_","beam_spot_free_loss_fsr_",
      "charged_ecal_free_loss_fsr_","neutral_ecal_free_loss_fsr_"};
    for(int i=0;i<4;++i)families[i].book(tree,prefixes[i]);
  }
  void reset() {for(auto& family:families)family=FsrFamilyTuple{};}
  void assignElectron(int i,const edm4hep::TrackState& ip,int state=1) {
    families.at(i).assignElectron(ip,state);
  }
  void assign(int i,const BeamSpotFsrResult& result,int actualMask,int state=2) {
    families.at(i).assign(result,actualMask,state);
  }
};
}
#endif
