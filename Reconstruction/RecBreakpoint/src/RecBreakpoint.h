#ifndef RECBREAKPOINT_ALGORITHM_H
#define RECBREAKPOINT_ALGORITHM_H

#include "GaudiKernel/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "podio/UserDataCollection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"

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
  DataHandle<edm4hep::TrackCollection> m_output{"BreakpointTracks", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_status{"BreakpointStatus", Gaudi::DataHandle::Writer, this};
  DataHandle<podio::UserDataCollection<std::int32_t>> m_outputIndex{"BreakpointOutputIndex", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCParticleCollection> m_truth{"MCParticle", Gaudi::DataHandle::Reader, this};

  Gaudi::Property<std::vector<int>> m_intervals{this, "BreakpointIntervals", {}};
  Gaudi::Property<double> m_meanLoss{this, "MeanLogLoss", 0.0};
  Gaudi::Property<double> m_sigmaLoss{this, "SigmaLogLoss", 0.05};
  Gaudi::Property<double> m_seedScale{this, "SeedScale", 1.0};
  Gaudi::Property<std::string> m_seedHitSelection{this, "SeedHitSelection", "FirstMiddleLast"};
  Gaudi::Property<std::string> m_backwardMode{this, "BackwardMode", "RTS"};
  Gaudi::Property<double> m_maxChi2{this, "MaxChi2PerHit", 1.e100};
  Gaudi::Property<bool> m_ms{this, "MSOn", true};
  Gaudi::Property<bool> m_eloss{this, "ElossOn", false};
  Gaudi::Property<bool> m_truthDiagnostics{this, "TruthDiagnostics", false};
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
  std::vector<int> m_breakpointIndex;
  std::string m_seedSelectionName;
  std::string m_backwardModeName;
  std::vector<int> m_seedHitIndices;
  std::vector<unsigned long long> m_hitCell;
  std::vector<double> m_hitR, m_hitZ, m_localChi2, m_filteredKappa, m_smoothedKappa;
  std::vector<double> m_filteredVariance, m_smoothedVariance;
  std::vector<double> m_backwardKappa, m_backwardVariance, m_backwardPredictedKappa;
  std::vector<double> m_backwardPredictedVariance, m_backwardChi2;
  std::vector<double> m_rtsLoss, m_rtsLossVariance;
  std::vector<double> m_priorLoss, m_localLoss, m_localVariance, m_fittedLoss, m_lossVariance, m_closure;
};
#endif
