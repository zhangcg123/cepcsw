#ifndef RecGsfTracking_GsfAlgorithm_h
#define RecGsfTracking_GsfAlgorithm_h

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"

#include "DetInterface/IGeomSvc.h"

#include <vector>
#include <map>

#include "TrackSystemSvc/HelixTrack.h"

class TKalDetCradle;
class DDCylinderMeasLayer;
class DDVMeasLayer;
class TVKalDetector;

/// Per-track comparison tuple for downstream analysis / plotting
struct TrackSummary {
  int iev = 0;
  int charge = 0;
  int nHits = 0;
  int nComps = 0;
  // truth
  double truth_pT   = 0, truth_eta = 0, truth_phi = 0, truth_p = 0;
  // LCIO seed
  double lcio_pT    = 0, lcio_eta  = 0, lcio_phi  = 0;
  double lcio_d0    = 0, lcio_z0   = 0, lcio_p    = 0;
  double lcio_chi2  = 0; int lcio_ndf  = 0;
  // GSF best component
  double gsf_pT     = 0, gsf_eta   = 0, gsf_phi   = 0;
  double gsf_d0     = 0, gsf_z0    = 0, gsf_p     = 0;
  double gsf_chi2   = 0; int gsf_ndf   = 0;
  // GSF diagnostics
  int    nSplits       = 0;   // number of BH split events
  int    nReductions   = 0;   // number of mixture reductions
  int    maxCompsEver  = 0;   // peak component count
  int    finalComps    = 0;   // components after final smoothing
  double bestWeight    = 0;   // weight of best component
  double meanWeight    = 0;   // average weight (should be ~1/N)
  double maxTX0Layer   = 0;   // largest single-layer t/X0
  double totalTX0      = 0;   // total accumulated radiation length
};

class RecGsfTracking : public Algorithm {
public:
  RecGsfTracking(const std::string& name, ISvcLocator* svc);
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  /// Access summaries after processing (for scripts / tests)
  const std::vector<TrackSummary>& summaries() const { return m_summaries; }

private:
  DataHandle<edm4hep::TrackCollection>       m_inputTracks{
      "CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection>       m_outputTracks{
      "GSFTracks", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCParticleCollection>  m_mcParticles{
      "MCParticle", Gaudi::DataHandle::Reader, this};

  SmartIF<IGeomSvc> m_geosvc;
  double m_field = 0.0;
  TKalDetCradle* m_cradle = nullptr;
  const DDCylinderMeasLayer* m_ipLayer = nullptr;
  std::vector<TVKalDetector*> m_detectors;

  Gaudi::Property<bool>   m_doMS{this,"MSOn",true};
  Gaudi::Property<bool>   m_doDEDX{this,"ElossOn",false};
  Gaudi::Property<int>    m_maxComponents{this,"MaxComponents",12};
  Gaudi::Property<double> m_bhSplitThresh{this,"BHSplitThreshold",1e-4};
  Gaudi::Property<bool>   m_isElectron{this,"ElectronHypothesis",true};
  Gaudi::Property<bool>   m_materialIPExtrap{this,"MaterialIPExtrapolation",false};
  Gaudi::Property<bool>   m_verboseDump{this,"VerboseDump",true};
  Gaudi::Property<double> m_kappaSeedCov{this,"KappaSeedCov",1e-7};

  int m_nEvt = 0;
  std::vector<TrackSummary> m_summaries;  // accumulated per-track data

  /// cellID → measurement layer index for O(1) hit lookup
  std::multimap<int, const DDVMeasLayer*> m_cellIDToLayer;
};

#endif
