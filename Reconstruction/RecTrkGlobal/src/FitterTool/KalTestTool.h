#ifndef KalTestTool_h
#define KalTestTool_h

#include "GaudiKernel/AlgTool.h"
#include "Tracking/ITrackFitterTool.h"
#include <vector>

namespace MarlinTrk {
  class IMarlinTrkSystem ;
}

class KalTestTool : public extends<AlgTool, ITrackFitterTool> {
 public:
  using extends::extends;
  //KalTestTool(void* p) { m_pAlgUsing=p; };
  
  virtual int Fit(edm4hep::MutableTrack track, std::vector<edm4hep::TrackerHit>& trackHits,
		  const decltype(edm4hep::TrackState::covMatrix)& covMatrix, double maxChi2perHit, bool backward = true) override;
  virtual int Fit(edm4hep::MutableTrack track, std::vector<edm4hep::TrackerHit>& trackHits,
                  edm4hep::TrackState trackState, double maxChi2perHit, bool backward = true) override;

  StatusCode initialize() override;
  StatusCode finalize() override;

  std::vector<std::pair<edm4hep::TrackerHit, double> >& GetHitsInFit() override {return m_hitsInFit;};
  std::vector<std::pair<edm4hep::TrackerHit, double> >& GetOutliers() override {return m_outliers;};
  void Clear() override {m_hitsInFit.clear(); m_outliers.clear();};

 private:
  Gaudi::Property<std::string>    m_fitterName{this, "Fitter", "KalTest"};
  Gaudi::Property<bool>           m_useQMS{this, "MSOn", true};
  Gaudi::Property<bool>           m_usedEdx{this, "EnergyLossOn", true};
  Gaudi::Property<bool>           m_useSmoothing{this, "Smooth", true};
  Gaudi::Property<double>         m_magneticField{this, "MagneticField", 0};
  
  MarlinTrk::IMarlinTrkSystem* m_factoryMarlinTrk = nullptr;

  void* m_pAlgUsing = nullptr;
  std::vector<std::pair<edm4hep::TrackerHit, double> > m_hitsInFit ;
  std::vector<std::pair<edm4hep::TrackerHit, double> > m_outliers ;
};

#endif
