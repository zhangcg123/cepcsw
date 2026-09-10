#ifndef RECBREAKPOINT_ALGORITHM_H
#define RECBREAKPOINT_ALGORITHM_H

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "GsfTruthEventData/G4MaterialStepCollection.h"
#include "GsfTruthEventData/SimTrackerHitG4StepLinkCollection.h"
#include "podio/UserDataCollection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "FreeLossTuple.h"
#include "FitPairTuple.h"

#include <cstdint>
#include <memory>
#include <vector>

class TFile;
class TTree;
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
  DataHandle<edm4hep::TrackCollection> m_output{"BreakpointTracksRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_backwardOutput{"BreakpointTracksBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_freeRTSOutput{"BreakpointTracksFreeLossRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_freeBackwardOutput{"BreakpointTracksFreeLossBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeResultStatus{"BreakpointFreeLossStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeRTSIndex{"BreakpointFreeLossRTSIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_freeBackwardIndex{"BreakpointFreeLossBackwardIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_truthRTSOutput{"BreakpointTracksTruthOverrideRTS", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::TrackCollection> m_truthBackwardOutput{"BreakpointTracksTruthOverrideBackwardFilter", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthResultStatus{"BreakpointTruthOverrideStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthRTSIndex{"BreakpointTruthOverrideRTSIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_truthBackwardIndex{"BreakpointTruthOverrideBackwardIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_status{"BreakpointStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_outputIndex{"BreakpointOutputIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_backwardOutputIndex{"BreakpointBackwardOutputIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCParticleCollection> m_truth{"MCParticle", Gaudi::DataHandle::Reader, this};
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
  Gaudi::Property<double> m_sigmaLoss{this, "SigmaLogLoss", 0.05};
  Gaudi::Property<double> m_seedScale{this, "SeedScale", 1.0};
  Gaudi::Property<double> m_backwardSeedScale{this, "BackwardSeedScale", 100.0};
  Gaudi::Property<std::string> m_seedHitSelection{this, "SeedHitSelection", "FirstMiddleLast"};
  Gaudi::Property<std::string> m_lossStateMode{this, "LossStateMode", "LocalMarginal"};
  Gaudi::Property<bool> m_freeLossFit{this, "FreeLossFit", false};
  Gaudi::Property<double> m_freeLossMax{this, "FreeLossMaxLogLoss", 1.0};
  Gaudi::Property<int> m_freeLossMaxCalls{this, "FreeLossMaxCallsPerStart", 180};
  Gaudi::Property<double> m_freeLossTolerance{this, "FreeLossTolerance", .001};
  Gaudi::Property<bool> m_freeLossCheck{this, "FreeLossCheckLikelihoods", false};
  Gaudi::Property<bool> m_enableTruthOverride{this, "TruthOverride", true};
  Gaudi::Property<double> m_maxChi2{this, "MaxChi2PerHit", 1.e100};
  Gaudi::Property<bool> m_ms{this, "MSOn", true};
  Gaudi::Property<bool> m_eloss{this, "ElossOn", false};
  Gaudi::Property<bool> m_truthDiagnostics{this, "TruthDiagnostics", false};
  Gaudi::Property<double> m_truthEndpointDistance{this, "TruthMaxEndpointDistance", 5.0};
  Gaudi::Property<bool> m_verbose{this, "VerboseDump", false};
  Gaudi::Property<bool> m_verifyReference{this, "VerifyKFReference", false};
  Gaudi::Property<std::vector<int>> m_selected{this, "SelectedEventIndices", {}};
  Gaudi::Property<std::string> m_tupleName{this, "OutputFile", "breakpoint_flat.root"};

  std::unique_ptr<breakpoint::BreakpointTrackSystem> m_system;
  double m_bz = 0;
  std::unique_ptr<TFile> m_file;
  TTree* m_tree = nullptr; // file owned
  int m_event = -1, m_trackIndex = -1, m_fitStatus = 0, m_hitCount = 0;
  double m_truthPt = 0, m_kfPt = 0, m_fitPt = 0, m_fitChi2 = 0;
  double m_referencePt = 0;
  double m_recordBackwardSeedScale = 100;
  breakpoint::FreeLossTuple m_freeLossTuple;
  breakpoint::FitPairTuple m_freeLossTracks;
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
  double m_backwardPt=0, m_backwardTotalChi2=0, m_smoothedTotalChi2=0, m_smoothedSeedChi2=0;
  double m_backwardSeedForwardChi2=0, m_backwardReferencePt=0;
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
  double m_truthRTSPt = 0, m_truthBackwardPt = 0;
  double m_truthForwardChi2 = 0, m_truthBackwardChi2 = 0, m_truthSmoothedChi2 = 0;
  int m_truthSmoothedStatus = 0;
  std::string m_truthSmoothedError;
  std::vector<double> m_truthForwardLocal, m_truthBackwardLocal, m_truthSmoothedLocal;
  std::vector<double> m_truthRTSParameters, m_truthRTSCovariance;
  std::vector<double> m_truthBackwardParameters, m_truthBackwardCovariance;
  // Distinguish truth-centered adjustable priors from historical fixed-b tuples.
  std::string m_truthLossTreatment = "PriorCenter";
  double m_truthPriorSigma = 0;
  std::vector<double> m_truthRTSLoss, m_truthRTSLossVariance;
  std::vector<double> m_truthBackwardLoss, m_truthBackwardLossVariance;
};
#endif
