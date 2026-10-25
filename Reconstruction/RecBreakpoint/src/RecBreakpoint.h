#ifndef RECBREAKPOINT_ALGORITHM_H
#define RECBREAKPOINT_ALGORITHM_H

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "GsfTruthEventData/G4MaterialStepCollection.h"
#include "GsfTruthEventData/SimTrackerHitG4StepLinkCollection.h"
#include "podio/UserDataCollection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "FreeLossTuple.h"
#include "FitPairTuple.h"
#include "LikelihoodTuple.h"
#include "CaloStateTuple.h"
#include "PhotonCaloProjection.h"
#include "RecoAssociationTuple.h"
#include "NeutralLossCandidate.h"
#include "ChargedEnergyCandidate.h"
#include "TruthBreakpointTuple.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

class TFile;
class TTree;
class TGeoNavigator;
namespace breakpoint { class BreakpointTrackSystem; }

/// Opt-in electron refitter; all new fitting helpers belong to this package.
class RecBreakpoint : public Algorithm {
public:
  RecBreakpoint(const std::string& name, ISvcLocator* locator);
  ~RecBreakpoint() override;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
private:
  DataHandle<edm4hep::TrackCollection> m_input{"CompleteTracks", Gaudi::DataHandle::Reader, this};
  // Cumulative RTS stages: beam, then charged, then neutral; ordinary then free-loss.
  std::array<DataHandle<edm4hep::TrackCollection>, 6> m_constraintOutputs{{
    {"BreakpointTracksBeamSpotRTS", Gaudi::DataHandle::Writer, this},
    {"BreakpointTracksBeamSpotFreeLossRTS", Gaudi::DataHandle::Writer, this},
    {"BreakpointTracksChargedEcalRTS", Gaudi::DataHandle::Writer, this},
    {"BreakpointTracksChargedEcalFreeLossRTS", Gaudi::DataHandle::Writer, this},
    {"BreakpointTracksNeutralEcalRTS", Gaudi::DataHandle::Writer, this},
    {"BreakpointTracksNeutralEcalFreeLossRTS", Gaudi::DataHandle::Writer, this}
  }};
  DataHandle<edm4hep::TrackCollection> m_output{"BreakpointTracksRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_backwardOutput{"BreakpointTracksBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_freeRTSOutput{"BreakpointTracksFreeLossRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_freeBackwardOutput{"BreakpointTracksFreeLossBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeResultStatus{"BreakpointFreeLossStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeRTSIndex{"BreakpointFreeLossRTSIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeBackwardIndex{"BreakpointFreeLossBackwardIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_truthRTSOutput{"BreakpointTracksTruthOverrideRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_diffuseRTSOutput{"BreakpointTracksDiffuseAugmentedRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_diffuseStatusOutput{"BreakpointDiffuseAugmentedStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_diffuseIndexOutput{"BreakpointDiffuseAugmentedIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_truthBackwardOutput{"BreakpointTracksTruthOverrideBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthResultStatus{"BreakpointTruthOverrideStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthRTSIndex{"BreakpointTruthOverrideRTSIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthBackwardIndex{"BreakpointTruthOverrideBackwardIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_status{"BreakpointStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_outputIndex{"BreakpointOutputIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_backwardOutputIndex{"BreakpointBackwardOutputIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoTrackParticleAssociationCollection> m_trackTruthAssociations{"CompleteTracksParticleAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_pidPfos{"CyberPFOPID", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ClusterCollection> m_ecalClusters{"EcalCluster", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_prePfos{"CyberPFOPreReclustering", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ClusterCollection> m_preEcalClusters{"EcalClusterPreReclustering", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::ClusterCollection> m_chargedEcalClusters{"", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::G4BremsPhotonCollection> m_bremsPhotons{"GsfG4BremsPhotons", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::G4BremsPhotonStepCollection> m_bremsPhotonSteps{"GsfG4BremsPhotonSteps", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::G4MaterialStepCollection> m_truthSteps{"GsfG4MaterialSteps", Gaudi::DataHandle::Reader, this};
  DataHandle<gsftruth::SimTrackerHitG4StepLinkCollection> m_truthLinks{"GsfSimTrackerHitG4StepLinks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_vxdAssociations{"VXDTrackerHitAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_itkbAssociations{"ITKBarrelTrackerHitAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_itkeAssociations{"ITKEndcapTrackerHitAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_tpcAssociations{"TPCTrackerHitAss", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_otkbAssociations{"OTKBarrelTrackerHitAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_otkeAssociations{"OTKEndcapTrackerHitAssociation", Gaudi::DataHandle::Reader, this};

  Gaudi::Property<std::vector<int>> m_intervals{this, "BreakpointIntervals", {}};
  Gaudi::Property<std::string> m_intervalSelection{this, "IntervalSelectionMode", "Truth"};
  Gaudi::Property<double> m_meanLoss{this, "MeanLogLoss", 0.0};
  Gaudi::Property<double> m_sigmaLoss{this, "SigmaLogLoss", 0.001};
  Gaudi::Property<double> m_seedScale{this, "SeedScale", 1.0};
  Gaudi::Property<double> m_backwardSeedScale{this, "BackwardSeedScale", 100.0};
  Gaudi::Property<bool> m_ordinaryBackwardFilter{this, "OrdinaryBackwardFilter", false};
  Gaudi::Property<bool> m_freeLossBackwardFilter{this, "FreeLossBackwardFilter", false};
  Gaudi::Property<bool> m_truthBackwardFilter{this, "TruthOverrideBackwardFilter", false};
  Gaudi::Property<std::string> m_seedHitSelection{this, "SeedHitSelection", "FirstMiddleLast"};
  Gaudi::Property<std::string> m_lossStateMode{this, "LossStateMode", "Persistent6D"};
  Gaudi::Property<bool> m_freeLossFit{this, "FreeLossFit", false};
  Gaudi::Property<bool> m_chargedEcalMeasurement{this, "ChargedEcalMeasurement", false};
  Gaudi::Property<std::string> m_ecalConstraintInput{this, "EcalConstraintInput", "PreReclustering"};
  Gaudi::Property<std::string> m_chargedEcalEnergyMode{this, "ChargedEcalEnergyMode", "ElectronOnly"};
  Gaudi::Property<bool> m_neutralEcalMeasurement{this, "NeutralEcalMeasurement", false};
  Gaudi::Property<double> m_chargedEcalStochasticError{this, "ChargedEcalStochasticError", 0.011};
  Gaudi::Property<double> m_chargedEcalConstantError{this, "ChargedEcalConstantError", 0.004};
  Gaudi::Property<bool> m_diffuseAugmentedRTS{this, "DiffuseAugmentedRTS", true};
  Gaudi::Property<double> m_neutralThetaWindow{this, "NeutralLossThetaWindowMrad", 10.0};
  Gaudi::Property<double> m_neutralPhiWindow{this, "NeutralLossPhiWindowMrad", 200.0};
  Gaudi::Property<double> m_neutralStochasticError{this, "NeutralLossStochasticError", 0.011};
  Gaudi::Property<double> m_neutralConstantError{this, "NeutralLossConstantError", 0.004};
  Gaudi::Property<bool> m_beamSpotConstraint{this, "BeamSpotConstraint", false};
  Gaudi::Property<double> m_beamSpotX{this, "BeamSpotX", 0.0};
  Gaudi::Property<double> m_beamSpotY{this, "BeamSpotY", 0.0};
  Gaudi::Property<double> m_beamSpotSigmaX{this, "BeamSpotSigmaX", 0.0145};
  Gaudi::Property<double> m_beamSpotSigmaY{this, "BeamSpotSigmaY", 3.6e-5};
  Gaudi::Property<double> m_freeLossMax{this, "FreeLossMaxLogLoss", 1.0};
  Gaudi::Property<int> m_freeLossMaxCalls{this, "FreeLossMaxCallsPerStart", 180};
  Gaudi::Property<double> m_freeLossTolerance{this, "FreeLossTolerance", .001};
  Gaudi::Property<bool> m_enableTruthOverride{this, "TruthOverride", true};
  Gaudi::Property<double> m_maxChi2{this, "MaxChi2PerHit", 1.e100};
  Gaudi::Property<bool> m_ms{this, "MSOn", true};
  Gaudi::Property<bool> m_eloss{this, "ElossOn", false};
  Gaudi::Property<double> m_truthEndpointDistance{this, "TruthMaxEndpointDistance", 5.0};
  Gaudi::Property<bool> m_verbose{this, "VerboseDump", false};
  Gaudi::Property<bool> m_verifyReference{this, "VerifyKFReference", false};
  Gaudi::Property<std::vector<int>> m_selected{this, "SelectedEventIndices", {}};
  Gaudi::Property<std::string> m_tupleName{this, "OutputFile", "breakpoint_flat.root"};

  std::unique_ptr<breakpoint::BreakpointTrackSystem> m_system;
  std::unique_ptr<TGeoNavigator> m_truthNavigator;
  breakpoint::TruthBreakpointTuple m_truthOwnership;
  double m_bz = 0;
  std::unique_ptr<TFile> m_file;
  TTree* m_tree = nullptr; // file owned
  TTree* m_neutralTree = nullptr; // file owned; one entry per processed event
  int m_event = -1, m_trackIndex = -1, m_fitStatus = 0, m_hitCount = 0;
  double m_kfPt = 0, m_kfPtError = 0, m_fitPt = 0, m_fitPtError = 0, m_fitChi2 = 0;
  breakpoint::TrackRecoTuple m_trackRecoTuple;
  breakpoint::NeutralPfoTuple m_neutralPfoTuple;
  double m_referencePt = 0, m_referencePtError = 0;
  double m_recordBackwardSeedScale = 100;
  std::array<int, 3> m_backwardRunFlags{};
  breakpoint::FreeLossTuple m_freeLossTuple;
  bool m_recordBeamSpotConstraint = false;
  breakpoint::BeamSpotSettings m_recordBeamSpotSettings;
  std::array<bool, 2> m_beamApplied{};
  std::array<breakpoint::BeamSpotMeasurement, 2> m_beamMeasurements;
  int m_neutralMeasurementStatus = 0; // 0 off, 1 no neutral, 2 applied, 3 no interval, -1 failed
  int m_ordinaryChargedStatus = 0, m_ordinaryNeutralStatus = 0; // 0 off, 1 unavailable/no breakpoint, 2 applied, -1 retry
  std::string m_ordinaryEcalError;
  breakpoint::LikelihoodTuple m_ordinaryLikelihood, m_freeLikelihood, m_truthLikelihood;
  breakpoint::FitPairTuple m_freeLossTracks;
  std::array<breakpoint::FitPairTuple, 6> m_constraintTracks;
  std::array<breakpoint::CaloStateTuple, 6> m_constraintCalo;
  std::array<breakpoint::LikelihoodTuple, 3> m_constraintOrdinaryLikelihood;
  std::array<breakpoint::FreeLossTuple, 3> m_constraintOptimization;
  // 0 off, 1 unavailable (previous-stage copy), 2 applied, -1 failed (previous-stage copy).
  std::array<int, 6> m_constraintStatus{};
  std::array<int, 6> m_constraintMask{}, m_constraintRequestedMask{}; // beam=1, charged=2, neutral=4
  std::array<std::string, 6> m_constraintError;
  // KF input plus seven parallel published breakpoint endpoint families.
  std::array<breakpoint::CaloStateTuple, 8> m_caloStates;
  std::array<breakpoint::PhotonCaloTuple, 2> m_photonCalo;
  // Collected independently for each input track, even when the ECAL fit is off.
  breakpoint::NeutralLossCandidate m_collectedNeutral;
  int m_collectedNeutralStatus = 0, m_collectedNeutralCount = 0;
  std::string m_collectedNeutralError;
  int m_chargedEcalMeasurementStatus = 0, m_chargedEcalMeasurementClusterCount = 0;
  double m_chargedEcalMeasurementEnergy = 0, m_chargedEcalMeasurementError = 0;
  std::string m_recordChargedEcalInput, m_recordChargedEcalEnergyMode;
  std::string m_recordChargedEcalEffectiveMode;
  std::string m_recordEcalConstraintInput, m_recordEcalConstraintPfos, m_recordEcalConstraintClusters;
  std::string m_chargedEcalSelectionError;
  std::vector<int> m_chargedEcalClusterIndices;
  int m_diffuseStatus = 0, m_diffuseIndex = -1;
  std::string m_diffuseError;
  double m_diffusePt = 0, m_diffusePtError = 0, m_diffuseB = 0, m_diffuseBVariance = 0;
  double m_diffuseEloss = 0, m_diffuseElossError = 0;
  std::vector<double> m_diffuseIPParameters, m_diffuseIPCovariance;
  std::vector<double> m_diffuseLocalChi2, m_diffuseFilteredB, m_diffuseSmoothedB;
  std::vector<int> m_diffuseHitIndex;
  std::vector<int> m_diffusePredictedUnresolved, m_diffuseFilteredUnresolved;
  std::vector<double> m_diffusePredictedMean, m_diffusePredictedCovariance;
  std::vector<double> m_diffuseFilteredMean, m_diffuseFilteredCovariance;
  std::vector<double> m_diffuseSmoothedMean, m_diffuseSmoothedCovariance;
  std::vector<double> m_diffuseTransport, m_diffuseNoise;
  std::vector<int> m_breakpointIndex;
  std::string m_seedSelectionName;
  std::string m_lossStateModeName;
  std::string m_intervalSelectionName, m_intervalSelectionError;
  int m_intervalSelectionStatus = 0, m_intervalTruthTrack = -1;
  double m_intervalTruthDistance = 0;
  std::vector<int> m_selectedIntervals;
  std::vector<double> m_intervalTruthLoss, m_intervalTruthZ;
  std::vector<int> m_persistentHits;
  std::vector<double> m_sixPredictedMean, m_sixPredictedCov, m_sixFilteredMean, m_sixFilteredCov;
  std::vector<double> m_sixSmoothedMean, m_sixSmoothedCov, m_sixTransport, m_sixNoise;
  std::vector<int> m_seedHitIndices;
  std::vector<unsigned long long> m_hitCell;
  std::vector<double> m_hitR, m_hitZ, m_localChi2, m_filteredKappa, m_smoothedKappa;
  std::vector<double> m_filteredVariance, m_smoothedVariance;
  std::vector<double> m_backwardKappa, m_backwardVariance, m_backwardPredictedKappa;
  std::vector<double> m_backwardPredictedVariance, m_backwardChi2;
  std::vector<double> m_rtsLoss, m_rtsLossVariance;
  std::vector<double> m_priorLoss, m_localLoss, m_localVariance, m_fittedLoss, m_lossVariance, m_closure;
  double m_backwardPt=0, m_backwardPtError=0, m_backwardTotalChi2=0, m_smoothedTotalChi2=0, m_smoothedSeedChi2=0;
  double m_backwardSeedForwardChi2=0, m_backwardReferencePt=0, m_backwardReferencePtError=0;
  int m_smoothedChi2Status=0;
  std::string m_smoothedChi2Error;
  std::vector<double> m_smoothedChi2, m_smoothedMeasurementChi2, m_smoothedProcessChi2, m_smoothedNativeChi2;
  std::vector<double> m_backwardLoss, m_backwardLossVariance;
  int m_truthOverrideStatus = 0, m_truthG4Track = -1;
  double m_truthMaxDistance = 0;
  std::string m_truthOverrideError;
  std::vector<int> m_truthIntervals, m_truthFirstStep, m_truthLastStep;
  std::vector<double> m_truthZ, m_truthB, m_truthMomentumBefore, m_truthEbremLoss, m_truthTX0;
  std::vector<double> m_truthStartFraction, m_truthEndFraction;
  // Parallel oracle endpoints. Status: 0 absent, 1 copied, 2 oracle, negative invalid.
  int m_truthResultCode = 0;
  double m_truthRTSPt = 0, m_truthRTSPtError = 0, m_truthBackwardPt = 0, m_truthBackwardPtError = 0;
  double m_truthForwardChi2 = 0, m_truthBackwardChi2 = 0, m_truthSmoothedChi2 = 0;
  int m_truthSmoothedStatus = 0;
  std::string m_truthSmoothedError;
  std::vector<double> m_truthForwardLocal, m_truthBackwardLocal, m_truthSmoothedLocal;
  std::vector<double> m_truthSmoothedMeasurement, m_truthSmoothedProcess, m_truthSmoothedNative;
  double m_truthSmoothedSeed = 0;
  std::vector<double> m_truthRTSParameters, m_truthRTSCovariance;
  std::vector<double> m_truthBackwardParameters, m_truthBackwardCovariance;
  // Distinguish truth-centered adjustable priors from historical fixed-b tuples.
  std::string m_truthLossTreatment = "PriorCenter";
  double m_truthPriorSigma = 0;
  std::vector<double> m_truthRTSLoss, m_truthRTSLossVariance;
  std::vector<double> m_truthBackwardLoss, m_truthBackwardLossVariance;
};
#endif
