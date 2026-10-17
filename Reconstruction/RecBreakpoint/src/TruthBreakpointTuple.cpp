#include "TruthBreakpointTuple.h"
#include "TTree.h"
namespace breakpoint {
void TruthBreakpointTuple::book(TTree& tree) {
  tree.Branch("truth_layer_association_status", &status);
  tree.Branch("truth_ebrem_no_unique_owner_count", &noUniqueOwner);
  tree.Branch("truth_ebrem_ineligible_count", &ineligible);
  tree.Branch("truth_ebrem_step", &step);
  tree.Branch("truth_ebrem_owner_status", &owner);
  tree.Branch("truth_ebrem_breakpoint_status", &eligibility);
  tree.Branch("truth_ebrem_owner_system", &system);
  tree.Branch("truth_ebrem_owner_layer", &layer);
  tree.Branch("truth_ebrem_breakpoint_hit", &hit);
  tree.Branch("truth_ebrem_owner_key", &ownerKey);
  tree.Branch("truth_ebrem_material_path", &materialPath);
  tree.Branch("truth_ebrem_candidate_layer_offset", &candidateOffset);
  tree.Branch("truth_ebrem_candidate_layer", &candidateLayer);
  tree.Branch("truth_ebrem_x", &x); tree.Branch("truth_ebrem_y", &y); tree.Branch("truth_ebrem_z", &z);
  tree.Branch("truth_ebrem_momentum_before", &momentumBefore);
  tree.Branch("truth_ebrem_momentum_loss", &loss);
}
void TruthBreakpointTuple::reset() { *this = TruthBreakpointTuple{}; }
void TruthBreakpointTuple::assign(const TruthBreakpointMatch& match) {
  reset(); status = 1; candidateOffset.push_back(0);
  for (const auto& e : match.emissions) {
    step.push_back(e.stepNumber); owner.push_back(static_cast<int>(e.owner));
    eligibility.push_back(static_cast<int>(e.status));
    system.push_back(e.system); layer.push_back(e.layer); hit.push_back(e.hitIndex);
    ownerKey.push_back(e.ownerKey); materialPath.push_back(e.materialPath);
    x.push_back(e.position.x); y.push_back(e.position.y); z.push_back(e.position.z);
    momentumBefore.push_back(e.momentumBefore); loss.push_back(e.momentumLoss);
    candidateLayer.insert(candidateLayer.end(), e.candidateLayers.begin(), e.candidateLayers.end());
    candidateOffset.push_back(static_cast<int>(candidateLayer.size()));
    if (e.owner != TruthLossOwner::Sensitive && e.owner != TruthLossOwner::Passive) ++noUniqueOwner;
    if (e.status != TruthBreakpointStatus::Eligible) ++ineligible;
  }
}
}
