#include "GsfComponent.h"

#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TVTrackHit.h"
#include "kaltest/KalTrackDim.h"

#include "DDKalTest/DDCylinderHit.h"
#include "DDKalTest/DDPlanarHit.h"

#include <algorithm>
#include <cmath>

namespace {
int covIndex5(int row, int col) {
  if (row < col) std::swap(row, col);
  return row * (row + 1) / 2 + col;
}
}

// ============================================================================

GsfComponent::~GsfComponent() {
  delete kaltrack;
}

THelicalTrack GsfComponent::helixAtLastSite(double bzTesla) const {
  if (continuationValid && bzTesla != 0.0) {
    const double alpha = bzTesla * 2.99792458e-4;
    const auto& ts = continuationState;
    return THelicalTrack(-ts.D0, ts.phi - M_PI / 2.0, ts.omega / alpha,
                         ts.Z0, ts.tanLambda, ts.referencePoint.x,
                         ts.referencePoint.y, ts.referencePoint.z, bzTesla);
  }
  return helixAtMeasurementSite(bzTesla);
}

THelicalTrack GsfComponent::helixAtMeasurementSite(double bzTesla) const {
  if (!kaltrack || kaltrack->GetEntriesFast() == 0) {
    TMatrixD s(5, 1);
    s.Zero();
    return THelicalTrack(s, TVector3(0, 0, 0), bzTesla);
  }
  auto& site = *dynamic_cast<const TKalTrackSite*>(kaltrack->Last());
  auto& state = dynamic_cast<TKalTrackState&>(site.GetCurState());
  return state.GetHelix();
}

TMatrixD GsfComponent::covAtLastSite(double bzTesla) const {
  TMatrixD c(5, 5);
  c.Zero();
  if (continuationValid && bzTesla != 0.0) {
    const double alpha = bzTesla * 2.99792458e-4;
    const double scale[5] = {-1.0, 1.0, 1.0 / alpha, 1.0, 1.0};
    for (int i = 0; i < 5; ++i)
      for (int j = 0; j < 5; ++j)
        c(i, j) = scale[i] * scale[j] *
                  continuationState.covMatrix[covIndex5(i, j)];
    return c;
  }
  if (!kaltrack || kaltrack->GetEntriesFast() == 0)
    return c;
  auto& site = *dynamic_cast<const TKalTrackSite*>(kaltrack->Last());
  auto& state = dynamic_cast<TKalTrackState&>(site.GetCurState());
  auto& kCov = state.GetCovMat();
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++)
      c(i, j) = kCov(i, j);
  return c;
}

bool GsfComponent::snapshotContinuation(double bzTesla, int location) {
  if (!kaltrack || kaltrack->GetEntriesFast() == 0 || bzTesla == 0.0)
    return false;
  auto* site = dynamic_cast<TKalTrackSite*>(kaltrack->Last());
  if (!site) return false;
  auto& state = dynamic_cast<TKalTrackState&>(site->GetCurState());
  const auto helix = state.GetHelix();
  const auto& pivot = helix.GetPivot();
  const double alpha = bzTesla * 2.99792458e-4;

  continuationState.location = location;
  continuationState.D0 = -helix.GetDrho();
  continuationState.phi = helix.GetPhi0() + M_PI / 2.0;
  while (continuationState.phi >= M_PI) continuationState.phi -= 2.0 * M_PI;
  while (continuationState.phi < -M_PI) continuationState.phi += 2.0 * M_PI;
  continuationState.omega = helix.GetKappa() * alpha;
  continuationState.Z0 = helix.GetDz();
  continuationState.tanLambda = helix.GetTanLambda();
  continuationState.referencePoint = {pivot.X(), pivot.Y(), pivot.Z()};
  for (auto& value : continuationState.covMatrix) value = 0.0;
  const double scale[5] = {-1.0, 1.0, alpha, 1.0, 1.0};
  const auto& cov = state.GetCovMat();
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j <= i; ++j)
      continuationState.covMatrix[covIndex5(i, j)] =
          scale[i] * scale[j] * cov(i, j);
  continuationValid = true;
  return true;
}

bool GsfComponent::setContinuationSurfaceState(const TMatrixD& mean,
                                                const TMatrixD& covariance,
                                                double bzTesla) {
  if (mean.GetNrows() < 5 || mean.GetNcols() != 1 ||
      covariance.GetNrows() < 5 || covariance.GetNcols() < 5 ||
      bzTesla == 0.0) {
    return false;
  }
  if (!continuationValid && !snapshotContinuation(bzTesla)) return false;

  const double alpha = bzTesla * 2.99792458e-4;
  continuationState.D0 = -mean(0, 0);
  continuationState.phi = mean(1, 0) + M_PI / 2.0;
  while (continuationState.phi >= M_PI) continuationState.phi -= 2.0 * M_PI;
  while (continuationState.phi < -M_PI) continuationState.phi += 2.0 * M_PI;
  continuationState.omega = mean(2, 0) * alpha;
  continuationState.Z0 = mean(3, 0);
  continuationState.tanLambda = mean(4, 0);
  for (auto& value : continuationState.covMatrix) value = 0.0;
  const double scale[5] = {-1.0, 1.0, alpha, 1.0, 1.0};
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j <= i; ++j)
      continuationState.covMatrix[covIndex5(i, j)] =
          scale[i] * scale[j] * covariance(i, j);
  continuationValid = true;
  return true;
}

// ---------------------------------------------------------------------------
// Deep clone: copies every site and every state
// ---------------------------------------------------------------------------
GsfComponent* GsfComponent::clone() const {
  auto* c = new GsfComponent();
  c->weight = weight;
  c->dominantLineageFraction = dominantLineageFraction;
  c->charge = charge;
  c->debugId = debugId;
  c->debugParentId = debugParentId;
  c->generation = generation;
  c->lineageNodeId = lineageNodeId;
  c->noRadiationLineage = noRadiationLineage;
  c->lastReverseProcessHit = lastReverseProcessHit;
  c->lastReverseProcessComponent = lastReverseProcessComponent;
  c->lastReverseProcessFraction = lastReverseProcessFraction;
  c->forwardProcessSignature = forwardProcessSignature;
  c->reverseProcessSignature = reverseProcessSignature;
  c->forwardProcessModeFractions = forwardProcessModeFractions;
  c->reverseProcessModeFractions = reverseProcessModeFractions;
  c->debugHistory = debugHistory;
  c->fitChi2 = fitChi2;
  c->continuationValid = continuationValid;
  c->continuationState = continuationState;
  c->pendingProcessJacobian = pendingProcessJacobian;
  c->smoothingSteps = smoothingSteps;
  c->smoothingNodeId = smoothingNodeId;
  c->smoothingSourceFractions = smoothingSourceFractions;
  c->smoothedInnerValid = smoothedInnerValid;
  c->smoothedInnerMean = smoothedInnerMean;
  c->smoothedInnerCovariance = smoothedInnerCovariance;
  c->kaltrack = new TKalTrack();
  c->kaltrack->SetOwner();

  for (int i = 0; i < kaltrack->GetEntriesFast(); i++) {
    auto* oldSite = dynamic_cast<TKalTrackSite*>(kaltrack->At(i));
    if (!oldSite) continue;

    const auto& oldHit = oldSite->GetHit();
    TVTrackHit* newHit = nullptr;
    if (auto* ch = dynamic_cast<const DDCylinderHit*>(&oldHit))
      newHit = new DDCylinderHit(*ch);
    else if (auto* ph = dynamic_cast<const DDPlanarHit*>(&oldHit))
      newHit = new DDPlanarHit(*ph);
    else
      continue;

    auto* newSite = new TKalTrackSite(*newHit, kSdim);
    newSite->SetPivot(oldSite->GetPivot());
    newSite->SetHitOwner();
    newSite->SetOwner();

    for (int j = 0; j < oldSite->GetEntries(); j++) {
      auto* oldSt = dynamic_cast<TKalTrackState*>(oldSite->At(j));
      if (!oldSt) continue;
      int type = (oldSt == oldSite->At(0)) ? TVKalSite::kPredicted
               : (oldSt == oldSite->At(1)) ? TVKalSite::kFiltered
               : TVKalSite::kSmoothed;
      newSite->Add(new TKalTrackState(
          static_cast<const TKalMatrix&>(*oldSt),
          oldSt->GetCovMat(), *newSite, type));
    }

    c->kaltrack->Add(newSite);
  }

  return c;
}
