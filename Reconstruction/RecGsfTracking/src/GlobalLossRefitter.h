#ifndef RecGsfTracking_GlobalLossRefitter_h
#define RecGsfTracking_GlobalLossRefitter_h

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "podio/UserDataCollection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "DetInterface/IGeomSvc.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class TKalDetCradle;
class DDCylinderMeasLayer;
class DDVMeasLayer;
class TVKalDetector;
namespace dd4hep { namespace rec { class MaterialManager; } }

/// Experimental all-inward-hit profile refitter.
///
/// The algorithm is deliberately separate from RecGsfTracking.  It reads
/// CompleteTracks directly and compares a no-radiation history with every
/// allowed one-radiative-interval BH hypothesis only after each hypothesis
/// has consumed all inward measurements.  Discrete histories are selected by
/// marginalizing the retained-momentum fraction under the BH mode prior; the
/// selected history's retained fraction is then continuously profiled.  There
/// is no KL reduction and no interaction with the production GSF output.
class RecGsfGlobalLossRefitter : public Algorithm {
public:
  RecGsfGlobalLossRefitter(const std::string& name, ISvcLocator* svc);
  ~RecGsfGlobalLossRefitter() override;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  DataHandle<edm4hep::TrackCollection> m_inputTracks{
      "CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection> m_outputTracks{
      "GlobalLossTracks", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_status{
      "GlobalLossStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_inputTrackIndex{
      "GlobalLossInputTrackIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_selectedInterval{
      "GlobalLossSelectedInterval", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_selectedMode{
      "GlobalLossSelectedMode", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_retainedFraction{
      "GlobalLossRetainedFraction", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_selectedTX0{
      "GlobalLossSelectedTX0", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_logLikelihood{
      "GlobalLossLogLikelihood", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_logPrior{
      "GlobalLossLogPrior", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_logPosteriorEvidence{
      "GlobalLossLogPosteriorEvidence", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_identityLogEvidence{
      "GlobalLossIdentityLogEvidence", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_bestRadiativeLogEvidence{
      "GlobalLossBestRadiativeLogEvidence", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<double>> m_radiativeLogBayesFactor{
      "GlobalLossRadiativeLogBayesFactor", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<bool> m_doMS{this, "MSOn", true};
  Gaudi::Property<bool> m_doDEDX{this, "ElossOn", false};
  Gaudi::Property<bool> m_isElectron{this, "ElectronHypothesis", true};
  Gaudi::Property<std::string> m_bhModel{
      this, "BHModel", "CEPC2GeV85StepConditioned"};
  Gaudi::Property<double> m_bhSplitThreshold{
      this, "BHSplitThreshold", 1.0e-4};
  Gaudi::Property<double> m_outerSeedCovarianceScale{
      this, "OuterSeedCovarianceScale", 100.0,
      "Full covariance scale applied to CompleteTracks AtLastHit"};
  Gaudi::Property<double> m_processSigmaWindow{
      this, "ProcessSigmaWindow", 3.0,
      "Half-width of each continuous retained-fraction profile window"};
  Gaudi::Property<int> m_profileGridPoints{
      this, "ProfileGridPoints", 9,
      "Odd coarse-grid size used within each BH mode"};
  Gaudi::Property<int> m_profileRefinementIterations{
      this, "ProfileRefinementIterations", 6,
      "Local interval-halving refinements after the coarse profile scan"};
  Gaudi::Property<double> m_minimumRetainedFraction{
      this, "MinimumRetainedFraction", 0.05,
      "Lower physical bound for the continuous retained fraction"};
  Gaudi::Property<double> m_minimumRadiativeLogBayesFactor{
      this, "MinimumRadiativeLogBayesFactor", 3.0,
      "Minimum best-radiative minus identity log evidence required to "
      "publish a radiative history"};
  Gaudi::Property<std::vector<int>> m_candidateIntervalIndices{
      this, "CandidateIntervalIndices", {},
      "Optional accepted-hit interval allow-list; empty scans all intervals"};
  Gaudi::Property<std::vector<int>> m_selectedEventIndices{
      this, "SelectedEventIndices", {},
      "Optional zero-based event allow-list"};
  Gaudi::Property<bool> m_verboseDump{
      this, "VerboseDump", false,
      "Print every interval/mode optimum and the selected profile"};

  SmartIF<IGeomSvc> m_geosvc;
  double m_field = 0.0;
  TKalDetCradle* m_cradle = nullptr;
  const DDCylinderMeasLayer* m_ipLayer = nullptr;
  std::vector<TVKalDetector*> m_detectors;
  std::multimap<int, const DDVMeasLayer*> m_cellIDToLayer;
  MarlinTrk::IMarlinTrkSystem* m_trackSystem = nullptr;
  dd4hep::rec::MaterialManager* m_materialManager = nullptr;
  int m_eventIndex = 0;
};

#endif
