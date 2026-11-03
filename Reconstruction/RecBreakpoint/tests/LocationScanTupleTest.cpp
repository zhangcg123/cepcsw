#include "HitLocationScan.h"
#include "edm4hep/TrackState.h"
#include "TTree.h"
#include <algorithm>
#include <cassert>

int main() {
  TTree tree("scan","scan");
  breakpoint::LocationFitScan tuple;
  tuple.book(tree);
  edm4hep::TrackState state{};
  state.location=edm4hep::TrackState::AtIP;
  state.D0=1; state.phi=2; state.omega=3; state.Z0=4; state.tanLambda=5; state.time=6;
  state.referencePoint={7,8,9};
  for (std::size_t i=0;i<state.covMatrix.size();++i) state.covMatrix[i]=i+.25f;
  tuple.appendIP(&state);
  auto inherited=state;
  state.phi=-2; state.tanLambda=-5;
  tuple.appendIP(&state);
  tuple.appendIP(&inherited);
  tuple.appendIP(nullptr);
  tuple.resolvedEcalSum={0,0,1,0};
  assert(tuple.ipParameters.size()==20 && tuple.ipCovariance.size()==84);
  assert(tuple.ipReferencePoint.size()==12 && tuple.ipTime.size()==4 && tuple.ipLocation.size()==4);
  tree.Fill();
  tuple.reset();
  assert(tuple.ipParameters.empty() && tuple.ipCovariance.empty() && tuple.ipReferencePoint.empty());
  assert(tuple.ipTime.empty() && tuple.ipLocation.empty());
  assert(tuple.resolvedEcalSum.empty());
  tuple.appendIP(nullptr);
  tree.Fill();

  std::vector<double> parameters,covariance,reference,time;
  std::vector<int> location,resolvedSum;
  auto* p=&parameters; auto* c=&covariance; auto* r=&reference; auto* t=&time; auto* l=&location;
  auto* sum=&resolvedSum;
  assert(tree.SetBranchAddress("location_scan_ip_parameters",&p)>=0);
  assert(tree.SetBranchAddress("location_scan_ip_covariance",&c)>=0);
  assert(tree.SetBranchAddress("location_scan_ip_reference_point",&r)>=0);
  assert(tree.SetBranchAddress("location_scan_ip_time",&t)>=0);
  assert(tree.SetBranchAddress("location_scan_ip_location",&l)>=0);
  assert(tree.SetBranchAddress("location_scan_resolved_ecal_sum_applied",&sum)>=0);
  tree.GetEntry(0);
  assert((resolvedSum==std::vector<int>{0,0,1,0}));
  assert(parameters.size()==20 && covariance.size()==84 && reference.size()==12);
  assert((std::vector<double>(parameters.begin(),parameters.begin()+5)==std::vector<double>{1,2,3,4,5}));
  assert(parameters[6]==-2 && parameters[9]==-5);
  assert(std::equal(parameters.begin(),parameters.begin()+5,parameters.begin()+10));
  for (int i=0;i<21;++i) assert(covariance[i]==i+.25 && covariance[i+42]==covariance[i]);
  assert((std::vector<double>(reference.begin(),reference.begin()+3)==std::vector<double>{7,8,9}));
  assert(time[0]==6 && location[0]==edm4hep::TrackState::AtIP);
  for (int i=15;i<20;++i) assert(std::isnan(parameters[i]));
  for (int i=63;i<84;++i) assert(std::isnan(covariance[i]));
  for (int i=9;i<12;++i) assert(std::isnan(reference[i]));
  assert(std::isnan(time[3]) && location[3]==-1);
  tree.GetEntry(1);
  assert(resolvedSum.empty());
  assert(parameters.size()==5 && covariance.size()==21 && reference.size()==3);
  assert(time.size()==1 && location.size()==1 && location[0]==-1);
  assert(std::all_of(parameters.begin(),parameters.end(),[](double x){return std::isnan(x);}));
  tree.ResetBranchAddresses();
}
