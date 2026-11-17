#include "BeamSpotFsrTuple.h"
#include "CalorimeterFitTuple.h"
#include "TFile.h"
#include "TLeaf.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
void checkSharedCalorimeterTuple() {
  TTree tree("shared","shared");
  breakpoint::CalorimeterFitTuple primary,ecal;
  primary.book(tree,"",true,true);ecal.book(tree,"ecal_only_",true,false);
  assert(tree.GetBranch("beam_spot_free_loss_rts_index"));
  assert(tree.GetBranch("ecal_only_charged_ecal_free_loss_rts_ip_parameters"));
  assert(tree.GetBranch("ecal_only_null_110_nll2"));
  assert(tree.GetBranch("ecal_only_fsr_only_110_energy"));
  assert(tree.GetBranch("ecal_only_neutral_ecal_free_loss_fsr_electron_p4"));
  assert(!tree.GetBranch("ecal_only_charged_ecal_free_loss_rts_index"));
  assert(!tree.GetBranch("ecal_only_beam_spot_free_loss_rts_pt"));
  assert(!tree.GetBranch("ecal_only_free_loss_fsr_status"));
  std::set<std::string> names;
  for (int i=0;i<tree.GetListOfBranches()->GetEntries();++i)
    assert(names.insert(tree.GetListOfBranches()->At(i)->GetName()).second);
  breakpoint::FreeLossSettings controls;
  primary.reset({true,true,true},true,controls,.001);
  ecal.reset({true,true,true},true,controls,.001);
  primary.fsrOnly110.status=2;primary.fsrOnly110.energy=3.;
  ecal.fsrOnly110.status=-2;ecal.fsrOnly110.error="missing ECAL";
  primary.status[3]=2;ecal.status[3]=-1;tree.Fill();
  ecal.reset({true,true,true},true,controls,.001);tree.Fill();
  tree.GetEntry(0);
  assert(tree.GetLeaf("ecal_only_fsr_only_110_status")->GetValue()==-2);
  assert(tree.GetLeaf("charged_ecal_free_loss_constraint_status")->GetValue()==2);
  tree.GetEntry(1);
  assert(tree.GetLeaf("ecal_only_fsr_only_110_status")->GetValue()==0);
  assert(std::isnan(tree.GetLeaf("ecal_only_fsr_only_110_energy")->GetValue()));
  assert(tree.GetLeaf("fsr_only_110_energy")->GetValue()==3.);
  assert(tree.GetLeaf("ecal_only_charged_ecal_free_loss_constraint_status")->GetValue()==0);
}
int main(int argc,char** argv) {
  assert(argc==2);
  checkSharedCalorimeterTuple();
  {
    TFile file(argv[1],"RECREATE");TTree tree("test","test");
    breakpoint::BeamSpotFsrTuple tuple;tuple.book(tree);tuple.reset();
    breakpoint::FsrFamilyTuple fsrOnly;fsrOnly.book(tree,"fsr_only_110_");
    assert(!tree.GetBranch("beam_fsr_pre_ip_parameters"));
    assert(tree.GetBranch("charged_ecal_free_loss_fsr_energy"));
    assert(tree.GetBranch("neutral_ecal_free_loss_fsr_electron_p4"));
    breakpoint::BeamSpotFsrResult result;result.energy=2.;result.variance=.3;
    breakpoint::TrackState electron;electron.mean.Zero();electron.mean(1,0)=.2;electron.mean(2,0)=.1;electron.mean(4,0)=.4;
    electron.covariance.UnitMatrix();electron.covariance*=1.e-6;
    TMatrixD joint(6,6);joint.UnitMatrix();joint*=1.e-6;joint(5,5)=.3;
    result.post=breakpoint::toEDM(electron,3.,1);result.p4=breakpoint::electronAndFsrP4(electron,2.,joint);
    for(int i=0;i<6;++i)for(int j=0;j<6;++j)result.jointIpEnergyCovariance.push_back(joint(i,j));
    tuple.assignElectron(0,result.post);tuple.families[0].error="undetermined";
    tuple.assign(2,result,3);tuple.assign(3,result,3,3);fsrOnly.assign(result,3);tree.Fill();
    tuple.reset();fsrOnly=breakpoint::FsrFamilyTuple{};tree.Fill();
    fsrOnly.status=-2;fsrOnly.error="missing ECAL";tree.Fill();tree.Write();
  }
  TFile file(argv[1],"READ");auto* tree=file.Get<TTree>("test");assert(tree);
  std::vector<double>* electron=nullptr;tree->SetBranchAddress("charged_ecal_free_loss_fsr_electron_p4",&electron);
  std::vector<double>* fsrElectron=nullptr;tree->SetBranchAddress("fsr_only_110_electron_p4",&fsrElectron);
  std::vector<double>* fsrCov=nullptr;tree->SetBranchAddress("fsr_only_110_joint_ip_energy_covariance",&fsrCov);
  std::vector<double>* fsrP4Cov=nullptr;tree->SetBranchAddress("fsr_only_110_joint_p4_covariance",&fsrP4Cov);
  std::string* error=nullptr;tree->SetBranchAddress("fsr_only_110_error",&error);
  tree->GetEntry(0);assert(tree->GetLeaf("charged_ecal_free_loss_fsr_energy")->GetValue()==2.);
  assert(tree->GetLeaf("neutral_ecal_free_loss_fsr_status")->GetValue()==3.);
  assert(tree->GetLeaf("free_loss_fsr_status")->GetValue()==1.);
  assert(std::isnan(tree->GetLeaf("free_loss_fsr_energy")->GetValue()));assert(electron->size()==4);
  assert(*fsrElectron==*electron && fsrCov->size()==36 && fsrP4Cov->size()==64);
  assert(tree->GetLeaf("fsr_only_110_status")->GetValue()==2.);
  assert(tree->GetLeaf("fsr_only_110_constraint_mask")->GetValue()==3.);
  assert(tree->GetLeaf("fsr_only_110_energy")->GetValue()==2.);
  tree->GetEntry(1);assert(tree->GetLeaf("charged_ecal_free_loss_fsr_status")->GetValue()==0.);
  assert(std::isnan(tree->GetLeaf("charged_ecal_free_loss_fsr_energy")->GetValue()));assert(electron->empty());
  assert(tree->GetLeaf("fsr_only_110_status")->GetValue()==0. && fsrElectron->empty() && fsrCov->empty() && fsrP4Cov->empty());
  assert(std::isnan(tree->GetLeaf("fsr_only_110_energy")->GetValue()));
  tree->GetEntry(2);assert(tree->GetLeaf("fsr_only_110_status")->GetValue()==-2.);
  assert(*error=="missing ECAL" && fsrElectron->empty() && fsrCov->empty());
  std::cout<<"PASS family branch names, valid/undetermined/retained status, persisted p4 and row reset\n";
}
