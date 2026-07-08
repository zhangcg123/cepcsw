#include "GsfComponent.h"

#include "kaltest/TKalTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TVTrackHit.h"
#include "kaltest/KalTrackDim.h"

#include "DDKalTest/DDCylinderHit.h"
#include "DDKalTest/DDPlanarHit.h"

// ============================================================================

GsfComponent::~GsfComponent() {
  delete kaltrack;
}

THelicalTrack GsfComponent::helixAtLastSite(double bzTesla) const {
  if (!kaltrack || kaltrack->GetEntriesFast() == 0) {
    TMatrixD s(5, 1);
    s.Zero();
    return THelicalTrack(s, TVector3(0, 0, 0), bzTesla);
  }
  auto& site = *dynamic_cast<const TKalTrackSite*>(kaltrack->Last());
  auto& state = dynamic_cast<TKalTrackState&>(site.GetCurState());
  return state.GetHelix();
}

TMatrixD GsfComponent::covAtLastSite() const {
  TMatrixD c(5, 5);
  c.Zero();
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

// ---------------------------------------------------------------------------
// Deep clone: copies every site and every state
// ---------------------------------------------------------------------------
GsfComponent* GsfComponent::clone() const {
  auto* c = new GsfComponent();
  c->weight = weight;
  c->charge = charge;
  c->debugId = debugId;
  c->debugHistory = debugHistory;
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
