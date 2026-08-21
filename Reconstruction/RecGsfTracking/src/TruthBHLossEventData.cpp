#include "TruthBHLossEventData.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace {

constexpr int kCompleteProvenanceStatus = (1 << 5) - 1;
constexpr double kFractionTolerance = 1.0e-6;

double squaredDistance(const edm4hep::Vector3d& lhs,
                       const edm4hep::Vector3d& rhs) {
  const double dx = lhs.x - rhs.x;
  const double dy = lhs.y - rhs.y;
  const double dz = lhs.z - rhs.z;
  return dx * dx + dy * dy + dz * dz;
}

edm4hep::Vector3d interpolatePosition(
    const edm4hep::Vector3d& from, const edm4hep::Vector3d& to,
    double fraction) {
  return {from.x + fraction * (to.x - from.x),
          from.y + fraction * (to.y - from.y),
          from.z + fraction * (to.z - from.z)};
}

}  // namespace

TruthBHLossEventData::ObjectKey TruthBHLossEventData::objectKey(
    const podio::ObjectID& id) {
  return {static_cast<int>(id.collectionID), static_cast<int>(id.index)};
}

double TruthBHLossEventData::momentumMagnitude(
    const edm4hep::Vector3f& momentum) {
  return std::sqrt(static_cast<double>(momentum.x) * momentum.x +
                   static_cast<double>(momentum.y) * momentum.y +
                   static_cast<double>(momentum.z) * momentum.z);
}

bool TruthBHLossEventData::prepare(
    const gsftruth::G4MaterialStepCollection* steps,
    const gsftruth::SimTrackerHitG4StepLinkCollection* links,
    const std::vector<const edm4hep::MCRecoTrackerAssociationCollection*>&
        associations,
    std::string& error) {
  m_steps.clear();
  m_linksBySimHit.clear();
  m_simHitsByRecoHit.clear();
  error.clear();

  if (!steps || !links) {
    error = "missing GsfG4MaterialSteps or GsfSimTrackerHitG4StepLinks";
    return false;
  }
  if (steps->empty() || links->empty()) {
    error = "embedded G4 step/link collections are empty";
    return false;
  }

  for (const auto& step : *steps) {
    const StepKey key{step.getTrackID(), step.getStepNumber()};
    StepRecord record;
    record.trackID = step.getTrackID();
    record.parentID = step.getParentID();
    record.stepNumber = step.getStepNumber();
    record.pdg = step.getPdg();
    record.processSubtype = step.getProcessSubtype();
    record.prePosition = step.getPrePosition();
    record.postPosition = step.getPostPosition();
    record.preMomentum = step.getPreMomentum();
    record.postMomentum = step.getPostMomentum();
    record.momentumLoss = step.getMomentumLoss();
    record.stepTX0 = step.getStepTX0();
    if (!m_steps.emplace(key, record).second) {
      std::ostringstream message;
      message << "duplicate embedded G4 step track=" << key.first
              << " step=" << key.second;
      error = message.str();
      return false;
    }
  }

  for (const auto& link : *links) {
    if (!link.getSimHit().isAvailable()) {
      error = "embedded provenance link has no SimTrackerHit relation";
      return false;
    }
    const auto simKey = objectKey(link.getSimHit().getObjectID());
    LinkRecord record;
    record.trackID = link.getTrackID();
    record.firstStepNumber = link.getFirstStepNumber();
    record.lastStepNumber = link.getLastStepNumber();
    record.hookStepNumber = link.getHookStepNumber();
    record.hookKind = link.getHookKind();
    record.hookFraction = link.getHookFraction();
    record.provenanceType = link.getProvenanceType();
    record.status = link.getStatus();

    // Legitimate collections can contain unresolved low-pT accumulation hits.
    // Preserve them in the lookup and reject only if the selected reconstructed
    // track actually uses one. Fully resolved links must be self-consistent.
    if (record.status == kCompleteProvenanceStatus) {
      if (record.firstStepNumber < 0 ||
          record.lastStepNumber < record.firstStepNumber ||
          record.hookStepNumber < record.firstStepNumber ||
          record.hookStepNumber > record.lastStepNumber ||
          !std::isfinite(record.hookFraction) ||
          record.hookFraction < 0.0 || record.hookFraction > 1.0) {
        error = "complete embedded provenance has invalid scalar bounds";
        return false;
      }
      const auto first = m_steps.find({record.trackID, record.firstStepNumber});
      const auto last = m_steps.find({record.trackID, record.lastStepNumber});
      const auto hook = m_steps.find({record.trackID, record.hookStepNumber});
      if (first == m_steps.end() || last == m_steps.end() ||
          hook == m_steps.end() || !link.getFirstStep().isAvailable() ||
          !link.getLastStep().isAvailable() ||
          !link.getHookStep().isAvailable() ||
          link.getFirstStep().getTrackID() != record.trackID ||
          link.getFirstStep().getStepNumber() != record.firstStepNumber ||
          link.getLastStep().getTrackID() != record.trackID ||
          link.getLastStep().getStepNumber() != record.lastStepNumber ||
          link.getHookStep().getTrackID() != record.trackID ||
          link.getHookStep().getStepNumber() != record.hookStepNumber) {
        error = "embedded provenance scalar fields and step relations disagree";
        return false;
      }
    }
    if (!m_linksBySimHit.emplace(simKey, record).second) {
      error = "multiple embedded provenance links target one SimTrackerHit";
      return false;
    }
  }

  for (const auto* collection : associations) {
    if (!collection) continue;
    for (const auto& association : *collection) {
      const auto reco = association.getRec();
      const auto sim = association.getSim();
      if (!reco.isAvailable() || !sim.isAvailable()) continue;
      const auto simKey = objectKey(sim.getObjectID());
      if (m_linksBySimHit.find(simKey) == m_linksBySimHit.end()) continue;
      m_simHitsByRecoHit[objectKey(reco.getObjectID())].insert(simKey);
    }
  }
  if (m_simHitsByRecoHit.empty()) {
    error = "no MCRecoTrackerAssociation reaches an embedded provenance link";
    return false;
  }
  return true;
}

bool TruthBHLossEventData::matchTrack(
    const std::vector<edm4hep::TrackerHit>& orderedHits,
    double maxEndpointDistance, bool collectMaterialIntervals,
    TruthBHLossEventDataMatch& match, std::string& error) const {
  match = TruthBHLossEventDataMatch{};
  error.clear();
  if (orderedHits.size() < 2) {
    error = "fewer than two accepted TrackerHits";
    return false;
  }

  std::vector<const LinkRecord*> hitLinks;
  hitLinks.reserve(orderedHits.size());
  for (std::size_t index = 0; index < orderedHits.size(); ++index) {
    const auto recoKey = objectKey(orderedHits[index].getObjectID());
    const auto associated = m_simHitsByRecoHit.find(recoKey);
    if (associated == m_simHitsByRecoHit.end() || associated->second.size() != 1) {
      std::ostringstream message;
      message << "accepted hit " << index << " has "
              << (associated == m_simHitsByRecoHit.end()
                      ? 0
                      : associated->second.size())
              << " provenance-bearing associated SimTrackerHits";
      error = message.str();
      return false;
    }
    const auto link = m_linksBySimHit.find(*associated->second.begin());
    if (link == m_linksBySimHit.end()) {
      error = "association points to an unavailable provenance link";
      return false;
    }
    if (link->second.status != kCompleteProvenanceStatus ||
        link->second.firstStepNumber < 0 ||
        link->second.lastStepNumber < link->second.firstStepNumber ||
        link->second.hookStepNumber < link->second.firstStepNumber ||
        link->second.hookStepNumber > link->second.lastStepNumber ||
        !std::isfinite(link->second.hookFraction) ||
        link->second.hookFraction < 0.0 ||
        link->second.hookFraction > 1.0) {
      std::ostringstream message;
      message << "accepted hit " << index
              << " has incomplete embedded provenance status="
              << link->second.status;
      error = message.str();
      return false;
    }
    if (index == 0) {
      match.g4TrackID = link->second.trackID;
    } else if (link->second.trackID != match.g4TrackID) {
      error = "accepted hits map to more than one Geant4 track";
      return false;
    }

    const auto hook = m_steps.find(
        {link->second.trackID, link->second.hookStepNumber});
    if (hook == m_steps.end()) {
      error = "hook step disappeared after event-data preparation";
      return false;
    }
    if (hook->second.parentID != 0 || std::abs(hook->second.pdg) != 11) {
      error = "associated provenance does not belong to a primary electron";
      return false;
    }
    const auto hookPosition = interpolatePosition(
        hook->second.prePosition, hook->second.postPosition,
        link->second.hookFraction);
    const auto& recoPosition = orderedHits[index].getPosition();
    const edm4hep::Vector3d endpoint{recoPosition.x, recoPosition.y,
                                     recoPosition.z};
    const double distance = std::sqrt(squaredDistance(endpoint, hookPosition));
    match.maxEndpointDistance = std::max(match.maxEndpointDistance, distance);
    if (!std::isfinite(distance) || distance > maxEndpointDistance) {
      std::ostringstream message;
      message << "accepted hit " << index
              << " is " << distance << " mm from its associated exact G4 hook"
              << " (limit " << maxEndpointDistance << " mm)";
      error = message.str();
      return false;
    }
    hitLinks.push_back(&link->second);
  }

  match.retainedFractions.reserve(orderedHits.size() - 1);
  if (collectMaterialIntervals)
    match.materialIntervals.reserve(orderedHits.size() - 1);
  for (std::size_t interval = 0; interval + 1 < hitLinks.size(); ++interval) {
    const auto& from = *hitLinks[interval];
    const auto& to = *hitLinks[interval + 1];
    const auto fromOrder = std::make_pair(from.hookStepNumber, from.hookFraction);
    const auto toOrder = std::make_pair(to.hookStepNumber, to.hookFraction);
    if (!(fromOrder < toOrder)) {
      std::ostringstream message;
      message << "accepted hit " << interval << "->" << interval + 1
              << " has nonmonotonic exact G4 hooks "
              << from.hookStepNumber << ":" << from.hookFraction << "->"
              << to.hookStepNumber << ":" << to.hookFraction;
      error = message.str();
      return false;
    }

    const auto fromStep = m_steps.find({from.trackID, from.hookStepNumber});
    if (fromStep == m_steps.end()) {
      error = "interval start hook step is unavailable";
      return false;
    }
    const double preP = momentumMagnitude(fromStep->second.preMomentum);
    const double postP = momentumMagnitude(fromStep->second.postMomentum);
    const double momentumBefore =
        preP + from.hookFraction * (postP - preP);
    if (!(momentumBefore > 0.0) || !std::isfinite(momentumBefore)) {
      error = "nonphysical momentum at exact interval start hook";
      return false;
    }

    TruthMaterialIntervalMatch materialInterval;
    materialInterval.firstStepNumber = from.hookStepNumber;
    materialInterval.lastStepNumber = to.hookStepNumber;
    materialInterval.startHookFraction = from.hookFraction;
    materialInterval.endHookFraction = to.hookFraction;
    materialInterval.startPosition = interpolatePosition(
        fromStep->second.prePosition, fromStep->second.postPosition,
        from.hookFraction);

    if (collectMaterialIntervals) {
      const auto toStep = m_steps.find({to.trackID, to.hookStepNumber});
      if (toStep == m_steps.end()) {
        error = "interval end hook step is unavailable";
        return false;
      }
      materialInterval.endPosition = interpolatePosition(
          toStep->second.prePosition, toStep->second.postPosition,
          to.hookFraction);
    }
    materialInterval.momentumBefore = momentumBefore;

    for (int stepNumber = from.hookStepNumber;
         stepNumber <= to.hookStepNumber; ++stepNumber) {
      const auto step = m_steps.find({from.trackID, stepNumber});
      if (step == m_steps.end()) {
        std::ostringstream message;
        message << "missing embedded G4 step " << stepNumber
                << " inside accepted-hit interval";
        error = message.str();
        return false;
      }
      double pieceStartFraction = 0.0;
      double pieceEndFraction = 1.0;
      if (stepNumber == from.hookStepNumber)
        pieceStartFraction = from.hookFraction;
      if (stepNumber == to.hookStepNumber)
        pieceEndFraction = to.hookFraction;
      const double stepFraction = pieceEndFraction - pieceStartFraction;
      if (collectMaterialIntervals &&
          (!std::isfinite(step->second.stepTX0) ||
           step->second.stepTX0 < 0.0 ||
           !std::isfinite(stepFraction) || stepFraction < 0.0)) {
        std::ostringstream message;
        message << "nonphysical Geant4 t/X0 piece for step " << stepNumber
                << ": stepTX0=" << step->second.stepTX0
                << " fraction=" << stepFraction;
        error = message.str();
        return false;
      }
      if (collectMaterialIntervals && stepFraction > kFractionTolerance) {
        materialInterval.truthTX0 += stepFraction * step->second.stepTX0;
        ++materialInterval.stepCount;
      }

      const bool postAfterStart =
          stepNumber > from.hookStepNumber ||
          from.hookFraction < 1.0 - kFractionTolerance;
      const bool postAtOrBeforeEnd =
          stepNumber < to.hookStepNumber ||
          to.hookFraction >= 1.0 - kFractionTolerance;
      if (postAfterStart && postAtOrBeforeEnd &&
          step->second.processSubtype == 3) {
        materialInterval.ebremLoss += step->second.momentumLoss;
      }
    }
    materialInterval.retainedFraction =
        1.0 - materialInterval.ebremLoss / momentumBefore;
    if ((collectMaterialIntervals &&
         (!std::isfinite(materialInterval.truthTX0) ||
          materialInterval.truthTX0 < 0.0)) ||
        !std::isfinite(materialInterval.ebremLoss) ||
        materialInterval.ebremLoss < 0.0 ||
        !std::isfinite(materialInterval.retainedFraction) ||
        materialInterval.retainedFraction <= 0.0 ||
        materialInterval.retainedFraction > 1.0) {
      std::ostringstream message;
      message << "nonphysical eBrem truth response for accepted hit "
              << interval << "->" << interval + 1
              << ": p_before=" << momentumBefore
              << " GeV, eBrem_loss=" << materialInterval.ebremLoss
              << " GeV, truth_tX0=" << materialInterval.truthTX0;
      error = message.str();
      return false;
    }
    match.retainedFractions.push_back(materialInterval.retainedFraction);
    if (collectMaterialIntervals)
      match.materialIntervals.push_back(materialInterval);
  }
  return true;
}
