#ifndef RECBREAKPOINT_CALORIMETER_FIT_TUPLE_H
#define RECBREAKPOINT_CALORIMETER_FIT_TUPLE_H

#include "BeamSpotFsrTuple.h"
#include "CaloStateTuple.h"
#include "FitPairTuple.h"
#include "FreeLossTuple.h"
#include "LikelihoodTuple.h"
#include "LocationFitScan.h"
#include "TTree.h"
#include <array>

namespace breakpoint {
// One calorimeter observation's results; inputs and tracker fits stay shared.
struct CalorimeterFitTuple {
  static constexpr const char* prefixes[] = {
    "beam_spot_ordinary_", "beam_spot_free_loss_",
    "charged_ecal_ordinary_", "charged_ecal_free_loss_",
    "neutral_ecal_ordinary_", "neutral_ecal_free_loss_"
  };
  LocationFitScan null110;
  FsrFamilyTuple fsrOnly110;
  double fsrOnlyEnergy=FsrFamilyTuple::missing, fsrOnlyError=FsrFamilyTuple::missing;
  int fsrOnlyResolvedSum=0, fsrOnlyDimensions=0;
  BeamSpotFsrTuple fsr;
  std::array<FitPairTuple,6> tracks;
  std::array<CaloStateTuple,6> calo;
  std::array<LikelihoodTuple,3> likelihood;
  std::array<FreeLossTuple,3> optimization;
  std::array<int,6> status{},mask{},requestedMask{};
  std::array<std::string,6> error;

  void book(TTree& tree, const std::string& prefix, bool fsrEnabled, bool primary) {
    null110.book(tree,prefix+"null_110_");
    if (fsrEnabled) {
      if (primary) fsr.book(tree);
      else for (int i=2;i<4;++i) fsr.families[i].book(tree,prefix+prefixes[2*i-1]+"fsr_");
      fsrOnly110.book(tree,prefix+"fsr_only_110_");
      tree.Branch((prefix+"fsr_only_110_ecal_observed_energy").c_str(),&fsrOnlyEnergy);
      tree.Branch((prefix+"fsr_only_110_ecal_observed_error").c_str(),&fsrOnlyError);
      tree.Branch((prefix+"fsr_only_110_resolved_ecal_sum_applied").c_str(),&fsrOnlyResolvedSum);
      tree.Branch((prefix+"fsr_only_110_measurement_dimensions").c_str(),&fsrOnlyDimensions);
    }
    for (std::size_t i=primary ? 0 : 2;i<tracks.size();++i) {
      const auto name=prefix+prefixes[i];
      tracks[i].book(tree,name,false,primary);
      calo[i].book(tree,name+"rts_");
      if (i%2==0) likelihood[i/2].book(tree,name+"likelihood_");
      tree.Branch((name+"constraint_status").c_str(),&status[i]);
      tree.Branch((name+"constraint_mask").c_str(),&mask[i]);
      tree.Branch((name+"constraint_requested_mask").c_str(),&requestedMask[i]);
      tree.Branch((name+"constraint_error").c_str(),&error[i]);
      if (i%2) optimization[i/2].book(tree,name+"optimizer_");
    }
  }
  void reset(const std::array<bool,3>& enabled, bool freeLoss,
             const FreeLossSettings& controls, double sigma) {
    null110.reset();fsr.reset();fsrOnly110=FsrFamilyTuple{};
    fsrOnlyEnergy=fsrOnlyError=FsrFamilyTuple::missing;
    fsrOnlyResolvedSum=fsrOnlyDimensions=0;
    for (std::size_t i=0;i<tracks.size();++i) {
      tracks[i].reset();calo[i].reset();
      if (i%2==0) likelihood[i/2].reset();
      if (i%2) optimization[i/2].reset(enabled[i/2] && freeLoss,controls,sigma);
      status[i]=mask[i]=requestedMask[i]=0;error[i].clear();
    }
  }
};
}
#endif
