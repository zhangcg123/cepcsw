#include "TruthLayerOwnership.h"

#include "DD4hep/DD4hepUnits.h"
#include "UTIL/BitField64.h"
#include "UTIL/ILDConf.h"
#include "TGeoManager.h"
#include "TGeoNavigator.h"
#include "TGeoVolume.h"
#include <algorithm>
#include <cmath>
#include <limits>

bool TruthBHLossEventData::matchBreakpoints(
    const std::vector<edm4hep::TrackerHit>& hits, double tolerance,
    TGeoNavigator& navigator, TruthBreakpointMatch& match, std::string& error) const {
  match = TruthBreakpointMatch{};
  if (!matchTrack(hits, tolerance, true, match.hooks, error)) return false;

  auto locate = [&](const StepRecord& step, double fraction) {
    const auto& a = step.prePosition;
    const auto& b = step.postPosition;
    const auto* node = navigator.FindNode((a.x+fraction*(b.x-a.x))*dd4hep::mm,
        (a.y+fraction*(b.y-a.y))*dd4hep::mm,
        (a.z+fraction*(b.z-a.z))*dd4hep::mm);
    if (!node) return std::make_pair(std::string{}, false);
    const double x0 = node->GetVolume()->GetMaterial()->GetRadLen()/dd4hep::mm;
    const bool compatible = std::isfinite(step.radiationLength) && step.radiationLength > 0 &&
        std::isfinite(x0) && std::abs(x0/step.radiationLength-1.) < 1.e-4;
    return std::make_pair(std::string(navigator.GetPath()), compatible);
  };
  auto endpoint = [&](const StepRecord& step) {
    const auto& a = step.prePosition;
    const auto& b = step.postPosition;
    const double length = std::hypot(std::hypot(b.x-a.x,b.y-a.y),b.z-a.z);
    // Approach the emission endpoint from the emitting volume, not the next volume.
    return locate(step, length > 0 ? 1.-std::min(0.25, 1.e-3/length) : 1.);
  };
  struct Sensor {
    ObjectKey sim;
    const LinkRecord* link;
    breakpoint::MaterialOwner owner;
  };
  std::vector<Sensor> sensors;
  for (const auto& entry : m_linksBySimHit) {
    const auto& link = entry.second;
    if (link.trackID != match.hooks.g4TrackID || link.status != 31) continue;
    const auto step = m_steps.find({link.trackID, link.hookStepNumber});
    if (step == m_steps.end()) continue;
    const auto location = locate(step->second, .5);
    auto owner = breakpoint::materialOwner(location.first);
    UTIL::BitField64 cell(lcio::ILDCellID0::encoder_string);
    cell.setValue(link.cellID);
    int system = cell["subdet"], layer = cell["layer"];
    const bool tpc = link.provenanceType >= 3 && link.provenanceType <= 5;
    if (tpc) {
      // TPC SD writes a one-based row number, NOT the silicon CellID bitfield.
      system = lcio::ILDDetID::TPC;
      layer = link.provenanceType == 5 ? -1 : static_cast<int>(link.cellID)-1;
    }
    if (!location.second || owner.kind != TruthLossOwner::Passive ||
        owner.system != system || owner.layer != layer) {
      // Exact sensitive provenance remains authoritative outside the audited
      // passive geometry too. Such cells cannot claim surrounding passive parts.
      owner.key = "cell:" + std::to_string(link.cellID);
    }
    if (tpc) owner.key = "TPC-row:" + std::to_string(layer);
    owner.kind = TruthLossOwner::Sensitive;
    owner.system = system; owner.layer = layer;
    sensors.push_back({entry.first, &link, owner});
  }
  std::map<ObjectKey, std::vector<int>> accepted;
  for (std::size_t i = 0; i < hits.size(); ++i)
    for (const auto& sim : m_simHitsByRecoHit.at(objectKey(hits[i].getObjectID())))
      accepted[sim].push_back(static_cast<int>(i));

  for (const auto& entry : m_steps) {
    const auto& step = entry.second;
    if (step.trackID != match.hooks.g4TrackID || step.processSubtype != 3) continue;
    TruthOwnedEmission emission;
    emission.stepNumber = step.stepNumber;
    emission.position = step.postPosition;
    emission.momentumBefore = momentumMagnitude(step.preMomentum);
    emission.momentumLoss = step.momentumLoss;
    const auto location = endpoint(step);
    emission.materialPath = location.first;
    auto owner = breakpoint::materialOwner(location.first);
    std::vector<const Sensor*> candidates;
    for (const auto& sensor : sensors)
      if (step.stepNumber >= sensor.link->firstStepNumber &&
          step.stepNumber <= sensor.link->lastStepNumber) candidates.push_back(&sensor);
    if (candidates.size() == 1) {
      owner = candidates.front()->owner;
    } else if (!candidates.empty()) {
      owner = {TruthLossOwner::Shared, location.first, -1, -1, {}};
      for (const auto* sensor : candidates) owner.candidates.push_back(sensor->owner.layer);
    } else if (!location.second) {
      owner = {TruthLossOwner::GeometryMismatch, location.first, -1, -1, {}};
    } else if (owner.kind == TruthLossOwner::Passive) {
      for (const auto& sensor : sensors) {
        if (sensor.owner.key != owner.key) continue;
        const int low = std::min(step.stepNumber, sensor.link->hookStepNumber);
        const int high = std::max(step.stepNumber, sensor.link->hookStepNumber);
        // Do not attach a returning primary's material loss to an earlier visit
        // to this assembly across an intervening different sensor assembly.
        const bool crossedOther = std::any_of(sensors.begin(), sensors.end(), [&](const auto& other) {
          return other.owner.key != owner.key && other.link->hookStepNumber > low &&
              other.link->hookStepNumber < high;
        });
        if (!crossedOther) candidates.push_back(&sensor);
      }
    }
    emission.owner = owner.kind; emission.ownerKey = owner.key;
    emission.system = owner.system; emission.layer = owner.layer;
    emission.candidateLayers = owner.candidates;
    if (owner.kind == TruthLossOwner::Sensitive || owner.kind == TruthLossOwner::Passive) {
      std::set<int> hitIndices;
      for (const auto* sensor : candidates) {
        const auto found = accepted.find(sensor->sim);
        if (found != accepted.end()) hitIndices.insert(found->second.begin(), found->second.end());
      }
      emission.status = hitIndices.empty() ? TruthBreakpointStatus::MissingHit :
          hitIndices.size() > 1 ? TruthBreakpointStatus::MultipleHits : TruthBreakpointStatus::Eligible;
      if (hitIndices.size() == 1) {
        emission.hitIndex = *hitIndices.begin();
        if (emission.hitIndex + 1 == static_cast<int>(hits.size()))
          emission.status = TruthBreakpointStatus::OutermostHit;
      }
    }
    if (!std::isfinite(emission.momentumLoss) || emission.momentumLoss < 0 ||
        !std::isfinite(emission.momentumBefore) || emission.momentumBefore <= emission.momentumLoss)
      emission.status = TruthBreakpointStatus::InvalidLoss;
    match.emissions.push_back(emission);
  }
  std::map<int, TruthBreakpointLoss> groups;
  for (const auto& emission : match.emissions) {
    if (emission.status != TruthBreakpointStatus::Eligible) continue;
    auto& group = groups[emission.hitIndex];
    group.hitIndex = emission.hitIndex;
    if (group.emissionCount++ == 0) group.momentumBefore = emission.momentumBefore;
    group.ebremLoss += emission.momentumLoss;
  }
  for (auto& entry : groups) {
    auto& group = entry.second;
    group.retainedFraction = 1.-group.ebremLoss/group.momentumBefore;
    if (!(group.retainedFraction > 0 && group.retainedFraction <= 1)) {
      for (auto& emission : match.emissions)
        if (emission.hitIndex == group.hitIndex && emission.status == TruthBreakpointStatus::Eligible)
          emission.status = TruthBreakpointStatus::InvalidLoss;
      continue;
    }
    match.breakpoints.push_back(group);
  }
  return true;
}
