#ifndef RecGsfTracking_GsfAlgorithm_h
#define RecGsfTracking_GsfAlgorithm_h

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "TruthBHLossTupleReader.h"

#include "DetInterface/IGeomSvc.h"

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include <cstdint>
#include <set>
#include <tuple>

#include "TrackSystemSvc/HelixTrack.h"

class TKalDetCradle;
class DDCylinderMeasLayer;
class DDVMeasLayer;
class TVKalDetector;
class ITrackSystemSvc;
namespace dd4hep { namespace rec { class MaterialManager; } }

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
  double maxTX0Layer   = 0;   // largest component-local outgoing path t/X0
  double totalTX0      = 0;   // sum of weighted outgoing path t/X0 values
};

class RecGsfTracking : public Algorithm {
public:
  RecGsfTracking(const std::string& name, ISvcLocator* svc);
  ~RecGsfTracking() override;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  /// Access summaries from the most recently processed event.
  const std::vector<TrackSummary>& summaries() const { return m_summaries; }

private:
  DataHandle<edm4hep::TrackCollection>       m_inputTracks{
      "CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection>       m_outputTracks{
      "GSFTracks", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection>       m_ecalConstrainedOutputTracks{
      "GSFTracksEcalConstrained", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ClusterCollection>     m_ecalClusters{
      "EcalCluster", Gaudi::DataHandle::Reader, this};
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
  Gaudi::Property<int>    m_reductionTargetComponents{this,"ReductionTargetComponents",0};
  Gaudi::Property<std::string> m_reductionMergeCost{
      this, "ReductionMergeCost", "SymmetricKL",
      "SymmetricKL or weighted Runnalls Gaussian merge cost"};
  Gaudi::Property<double> m_componentWeightCutoff{this,"ComponentWeightCutoff",1e-4};
  Gaudi::Property<bool>   m_protectIdentityLineage{
      this, "ProtectIdentityLineage", true,
      "Preserve an exact no-radiation lineage through cutoff and reduction"};
  Gaudi::Property<double> m_bhSplitThresh{this,"BHSplitThreshold",1e-4};
  Gaudi::Property<bool>   m_isElectron{this,"ElectronHypothesis",true};
  Gaudi::Property<bool>   m_materialIPExtrap{this,"MaterialIPExtrapolation",false};
  Gaudi::Property<bool>   m_reverseFiltering{this,"ReverseFiltering",false};
  Gaudi::Property<bool>   m_cmsGsfSmoothing{
      this,"CmsGsfSmoothing",false,
      "Run a CMSSW-like backward GSF seeded from the final forward prediction, "
      "with collapsed-moment smoothing and innermost backward-filter output"};
  Gaudi::Property<double> m_cmsErrorRescaling{
      this,"CmsErrorRescaling",100.0,
      "Multiplicative covariance scaling for the CMSSW-like backward seed"};
  Gaudi::Property<std::string> m_reverseOutputMode{
      this, "ReverseOutputMode", "BestBranch",
      "WeightedMean or BestBranch output from the reverse filter"};
  Gaudi::Property<std::string> m_reverseSelectionMode{
      this, "ReverseSelectionMode", "AggregateWeight",
      "AggregateWeight, DominantLineage, or default-off SurfaceConsistency "
      "final reverse-branch selection"};
  Gaudi::Property<double> m_surfaceConsistencyUninformativeFloor{
      this, "SurfaceConsistencyUninformativeFloor", 0.05,
      "Lower bound on the bounded surface-consistency likelihood; 0.05 "
      "caps its final-selection Bayes factor at 20"};
  Gaudi::Property<std::string> m_reverseInitialWeightMode{
      this, "ReverseInitialWeightMode", "ForwardPosterior",
      "ForwardPosterior or Uniform diagnostic reverse-start weights"};
  Gaudi::Property<double> m_reverseKappaSeedCov{
      this, "ReverseKappaSeedCov", 100.0,
      "Multiplicative covariance scaling for every full-mixture reverse seed "
      "component (legacy property name)"};
  Gaudi::Property<bool> m_ecalComponentConstraint{
      this, "EcalComponentConstraint", false,
      "Default-off two-sided ECAL likelihood constraint on final reverse "
      "BestBranch selection; the unconstrained GSFTracks output is preserved"};
  Gaudi::Property<double> m_ecalConstraintRatioThreshold{
      this, "EcalConstraintRatioThreshold", 1.1,
      "Activate the ECAL branch constraint when max(p/E,E/p) exceeds this "
      "ratio"};
  Gaudi::Property<double> m_ecalConstraintLogPSigma{
      this, "EcalConstraintLogPSigma", 0.15,
      "Width of the Gaussian component likelihood in log(p/E)"};
  Gaudi::Property<double> m_ecalConstraintLikelihoodFloor{
      this, "EcalConstraintLikelihoodFloor", 0.05,
      "Lower bound on the ECAL component likelihood, limiting its Bayes factor"};
  Gaudi::Property<double> m_ecalConstraintPhiWindow{
      this, "EcalConstraintPhiWindow", 0.10,
      "Absolute azimuth window around the extrapolated outer GSF direction "
      "used to sum EcalCluster energy"};
  Gaudi::Property<double> m_ecalConstraintThetaWindow{
      this, "EcalConstraintThetaWindow", 0.10,
      "Absolute polar-angle window around the extrapolated outer GSF "
      "direction used to sum EcalCluster energy"};
  Gaudi::Property<bool>   m_gaussianSumSmoothing{this,"GaussianSumSmoothing",false};
  Gaudi::Property<bool>   m_verboseDump{this,"VerboseDump",false};
  Gaudi::Property<bool>   m_verboseSplitDump{this,"VerboseSplitDump",false};
  Gaudi::Property<bool>   m_componentDebugDump{this,"ComponentDebugDump",false};
  Gaudi::Property<bool>   m_surfaceLineageMassDump{
      this,"SurfaceLineageMassDump",false,
      "Opt-in propagation and verbose dump of aggregate BH-mode mass by surface"};
  Gaudi::Property<int>    m_componentDebugMaxHistory{this,"ComponentDebugMaxHistory",240};
  Gaudi::Property<std::vector<int>> m_selectedEventIndices{this,"SelectedEventIndices",{}};
  Gaudi::Property<double> m_kappaSeedCov{this,"KappaSeedCov",1e-7};
  Gaudi::Property<std::string> m_bhModel{
      this, "BHModel", "CEPC2GeV85StepConditioned"};
  Gaudi::Property<bool> m_truthBHLossOverride{
      this, "TruthBHLossOverride", false,
      "Default-off diagnostic replacing each executed BH response on every "
      "selected track with one exact externally supplied Geant4 eBrem "
      "retained-momentum fraction"};
  Gaudi::Property<std::string> m_truthBHLossSource{
      this, "TruthBHLossSource", "CSV",
      "CSV or G4StepTuple source used only by TruthBHLossOverride"};
  Gaudi::Property<std::string> m_truthBHLossInput{
      this, "TruthBHLossInput", "",
      "Strict consecutive-hit CSV or GsfMaterialStepRecorder ROOT tuple used "
      "only by TruthBHLossOverride"};
  Gaudi::Property<int> m_truthBHLossInputTrackIndex{
      this, "TruthBHLossInputTrackIndex", 0,
      "CompleteTracks index receiving the primary-electron G4StepTuple truth "
      "oracle; other tracks use the configured BH model"};
  Gaudi::Property<double> m_truthBHLossMaxEndpointDistance{
      this, "TruthBHLossMaxEndpointDistance", 5.0,
      "Maximum allowed distance in mm between a runtime accepted hit and its "
      "nearest ordered Geant4 sensitive-midpoint anchor"};
  Gaudi::Property<bool> m_counterfactualLossScan{
      this, "CounterfactualLossScan", false,
      "Default-off likelihood-only scan of trial losses at a configured truth "
      "surface and one surface inward; never enters the live GSF mixture"};
  Gaudi::Property<std::string> m_counterfactualTruthTransitionMap{
      this, "CounterfactualTruthTransitionMap", "",
      "Comma-separated event:transition map used only by CounterfactualLossScan"};
  Gaudi::Property<std::vector<double>> m_counterfactualLossFractions{
      this, "CounterfactualLossFractions", {0.04, 0.05, 0.06, 0.07, 0.08,
                                             0.09, 0.10, 0.12},
      "Trial fractional momentum losses for CounterfactualLossScan"};
  Gaudi::Property<double> m_counterfactualLossVariance{
      this, "CounterfactualLossVariance", 2.0e-4,
      "Retained-momentum-fraction variance assigned to trial scan components"};
  Gaudi::Property<std::string> m_outputMode{this,"GSFOutputMode","BestBranch"};
  Gaudi::Property<std::string> m_materialPathMode{
      this, "MaterialPathMode", "DD4hepBetweenSurfaces",
      "Forward/reverse material assignment: CurrentSurface or "
      "DD4hepBetweenSurfaces"};
  Gaudi::Property<std::string> m_materialTransitionCSV{
      this, "MaterialTransitionCSV", "",
      "Optional component-local outgoing-surface material transition CSV"};
  Gaudi::Property<std::string> m_materialBHAuditCSV{
      this, "MaterialBHAuditCSV", "",
      "Optional structured seed/forward/reverse material candidate and "
      "executed Bethe-Heitler call CSV"};
  MarlinTrk::IMarlinTrkSystem* m_gsfMarlinTrkSystem = nullptr;

  std::ofstream m_materialTransitionStream;
  std::ofstream m_materialBHAuditStream;
  std::uint64_t m_materialBHNextCallId = 0;
  using TruthBHLossKey =
      std::tuple<int, int, int, int, std::uint64_t, std::uint64_t>;
  std::map<TruthBHLossKey, double> m_truthBHRetainedFractions;
  std::set<std::pair<int, int>> m_truthBHSelectedTracks;
  std::unique_ptr<TruthBHLossTupleReader> m_truthBHLossTupleReader;
  bool m_truthBHLossUsesTuple = false;
  std::uint64_t m_truthBHLossOverrideCalls = 0;
  std::uint64_t m_truthBHLossPassthroughTracks = 0;
  std::uint64_t m_truthBHLossTupleTracks = 0;
  double m_truthBHLossMaxObservedEndpointDistance = 0.0;
  dd4hep::rec::MaterialManager* m_materialManager = nullptr;

  int m_nEvt = 0;
  std::vector<TrackSummary> m_summaries;  // current-event per-track data

  /// cellID → measurement layer index for O(1) hit lookup
  std::multimap<int, const DDVMeasLayer*> m_cellIDToLayer;
};

#endif
