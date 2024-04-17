#include "KalTestTool.h"

#include "TrackSystemSvc/ITrackSystemSvc.h"
#include "TrackSystemSvc/MarlinTrkUtils.h"
#include "TrackSystemSvc/IMarlinTrack.h"
#include "DetInterface/IGeomSvc.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"

#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackState.h"
#include "edm4hep/MutableTrack.h"

DECLARE_COMPONENT(KalTestTool)

StatusCode KalTestTool::initialize() {
  StatusCode sc;
  always() << m_fitterName << endmsg;
  if (m_fitterName=="KalTest") {
    auto _trackSystemSvc = service<ITrackSystemSvc>("TrackSystemSvc");
    if (!_trackSystemSvc) {
      error() << "Failed to find TrackSystemSvc ..." << endmsg;
      return StatusCode::FAILURE;
    }
    m_factoryMarlinTrk = _trackSystemSvc->getTrackSystem(this);
    m_factoryMarlinTrk->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useQMS, m_useQMS);
    m_factoryMarlinTrk->setOption(MarlinTrk::IMarlinTrkSystem::CFG::usedEdx, m_usedEdx);
    m_factoryMarlinTrk->setOption(MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing, m_useSmoothing);
    m_factoryMarlinTrk->init();
  }
  else {
    error() << "fitter " << m_fitterName << " has not been imported" << endmsg;
    return StatusCode::FAILURE;
  }

  auto _geomSvc = service<IGeomSvc>("GeomSvc");
  if ( !_geomSvc ) {
    error() << "Failed to find GeomSvc ..." << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_magneticField.value()==0) {
    const dd4hep::Direction& field = _geomSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
    double Bz = field.z()/dd4hep::tesla;
    if (Bz==0) {
      error() << "magnetic field = 0, KalmanFilter cannot run" << endmsg;
      return StatusCode::FAILURE;
    }
    m_magneticField = Bz;
  }

  return sc;
}

StatusCode KalTestTool::finalize() {
  StatusCode sc;
  return sc;
}

int KalTestTool::Fit(edm4hep::MutableTrack track, std::vector<edm4hep::TrackerHit>& trackHits,
			  const decltype(edm4hep::TrackState::covMatrix)& covMatrix, double maxChi2perHit, bool backward) {
  if (m_hitsInFit.size()!=0 || m_outliers.size()!=0) {
    error() << "Important! vector not clear, still store the data of last event!" << endmsg;
    return 0;
  }

  if (m_fitterName=="KalTest") {
    debug() << "start..." << endmsg;
    std::shared_ptr<MarlinTrk::IMarlinTrack> marlinTrack(m_factoryMarlinTrk->createTrack());
    debug() << "created MarlinKalTestTrack" << endmsg;
    int status = MarlinTrk::createFinalisedLCIOTrack(marlinTrack.get(), trackHits, &track, backward, covMatrix, m_magneticField, maxChi2perHit);

    marlinTrack->getHitsInFit(m_hitsInFit);
    marlinTrack->getOutliers(m_outliers);
    return status;
  }
  
  error() << "Don't support the Fitter " << m_fitterName << endmsg;
  return 0;
}

int KalTestTool::Fit(edm4hep::MutableTrack track, std::vector<edm4hep::TrackerHit>& trackHits,
                          edm4hep::TrackState trackState, double maxChi2perHit, bool backward) {
  if (m_hitsInFit.size()!=0 || m_outliers.size()!=0) {
    error() << "Important! vector not clear, still store the data of last event!" << endmsg;
    return 0;
  }

  if (m_fitterName=="KalTest") {
    debug() << "start..." << endmsg;
    std::shared_ptr<MarlinTrk::IMarlinTrack> marlinTrack(m_factoryMarlinTrk->createTrack());
    debug() << "created MarlinKalTestTrack" << endmsg;
    int status = MarlinTrk::createFinalisedLCIOTrack(marlinTrack.get(), trackHits, &track, backward, &trackState, m_magneticField, maxChi2perHit);

    marlinTrack->getHitsInFit(m_hitsInFit);
    marlinTrack->getOutliers(m_outliers);
    return status;
  }

  error() << "Don't support the Fitter " << m_fitterName << endmsg;
  return 0;
}
