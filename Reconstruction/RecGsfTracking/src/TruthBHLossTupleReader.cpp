#include "TruthBHLossTupleReader.h"

#include <TFile.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <utility>

namespace {

bool isTpcUpper(const std::string& path) {
  return path.find("/TPC_upperlayer_log_") != std::string::npos;
}

bool isTpcLower(const std::string& path) {
  return path.find("/TPC_lowerlayer_log_") != std::string::npos;
}

template <typename T>
bool sameSize(const std::vector<T>* values, std::size_t size) {
  return values && values->size() == size;
}

}  // namespace

struct TruthBHLossTupleReader::Impl {
  std::unique_ptr<TFile> file;
  TTree* tree = nullptr;
  int eventId = -1;

  std::vector<int>* trackId = nullptr;
  std::vector<int>* parentId = nullptr;
  std::vector<int>* isPrimary = nullptr;
  std::vector<int>* trackStepNumber = nullptr;
  std::vector<int>* recordedStepIndex = nullptr;
  std::vector<int>* pdg = nullptr;
  std::vector<int>* stepStatusPre = nullptr;
  std::vector<int>* processSubtype = nullptr;
  std::vector<int>* preSensitive = nullptr;

  std::vector<float>* preX = nullptr;
  std::vector<float>* preY = nullptr;
  std::vector<float>* preZ = nullptr;
  std::vector<float>* postX = nullptr;
  std::vector<float>* postY = nullptr;
  std::vector<float>* postZ = nullptr;
  std::vector<float>* preP = nullptr;
  std::vector<float>* postP = nullptr;
  std::vector<float>* loss = nullptr;
  std::vector<float>* trackLengthPre = nullptr;
  std::vector<float>* trackLengthPost = nullptr;
  std::vector<std::string>* preTouchablePath = nullptr;
};

TruthBHLossTupleReader::TruthBHLossTupleReader()
    : m_impl(std::make_unique<Impl>()) {}

TruthBHLossTupleReader::~TruthBHLossTupleReader() = default;

bool TruthBHLossTupleReader::open(const std::string& path, std::string& error) {
  auto file = std::unique_ptr<TFile>(TFile::Open(path.c_str(), "READ"));
  if (!file || file->IsZombie()) {
    error = "cannot open Geant4 material tuple " + path;
    return false;
  }
  auto* tree = dynamic_cast<TTree*>(file->Get("g4step_tuple"));
  if (!tree) {
    error = "missing g4step_tuple in " + path;
    return false;
  }

  const std::vector<std::string> requiredBranches{
      "event_id", "track_id", "parent_id", "is_primary",
      "track_step_number", "recorded_step_index", "pdg",
      "step_status_pre", "process_subtype", "pre_sensitive",
      "pre_x", "pre_y", "pre_z", "post_x", "post_y", "post_z",
      "pre_p", "post_p", "loss", "track_length_pre",
      "track_length_post", "pre_touchable_path"};
  for (const auto& branch : requiredBranches) {
    if (!tree->GetBranch(branch.c_str())) {
      error = "g4step_tuple is missing required branch " + branch;
      return false;
    }
  }

  tree->SetBranchStatus("*", 0);
  for (const auto& branch : requiredBranches)
    tree->SetBranchStatus(branch.c_str(), 1);

  auto& data = *m_impl;
  data.file = std::move(file);
  data.tree = tree;
  tree->SetBranchAddress("event_id", &data.eventId);
  tree->SetBranchAddress("track_id", &data.trackId);
  tree->SetBranchAddress("parent_id", &data.parentId);
  tree->SetBranchAddress("is_primary", &data.isPrimary);
  tree->SetBranchAddress("track_step_number", &data.trackStepNumber);
  tree->SetBranchAddress("recorded_step_index", &data.recordedStepIndex);
  tree->SetBranchAddress("pdg", &data.pdg);
  tree->SetBranchAddress("step_status_pre", &data.stepStatusPre);
  tree->SetBranchAddress("process_subtype", &data.processSubtype);
  tree->SetBranchAddress("pre_sensitive", &data.preSensitive);
  tree->SetBranchAddress("pre_x", &data.preX);
  tree->SetBranchAddress("pre_y", &data.preY);
  tree->SetBranchAddress("pre_z", &data.preZ);
  tree->SetBranchAddress("post_x", &data.postX);
  tree->SetBranchAddress("post_y", &data.postY);
  tree->SetBranchAddress("post_z", &data.postZ);
  tree->SetBranchAddress("pre_p", &data.preP);
  tree->SetBranchAddress("post_p", &data.postP);
  tree->SetBranchAddress("loss", &data.loss);
  tree->SetBranchAddress("track_length_pre", &data.trackLengthPre);
  tree->SetBranchAddress("track_length_post", &data.trackLengthPost);
  tree->SetBranchAddress("pre_touchable_path", &data.preTouchablePath);

  if (tree->BuildIndex("event_id") < 0) {
    error = "cannot index g4step_tuple by event_id";
    data = Impl{};
    return false;
  }
  return true;
}

long long TruthBHLossTupleReader::entries() const {
  return m_impl && m_impl->tree ? m_impl->tree->GetEntries() : 0;
}

bool TruthBHLossTupleReader::readPrimaryElectronIntervals(
    int eventIndex, std::vector<TruthBHLossSurfaceInterval>& intervals,
    std::string& error) {
  intervals.clear();
  if (!m_impl || !m_impl->tree) {
    error = "Geant4 material tuple reader is not open";
    return false;
  }
  auto& data = *m_impl;
  const auto entry = data.tree->GetEntryNumberWithIndex(eventIndex);
  if (entry < 0 || data.tree->GetEntry(entry) <= 0 ||
      data.eventId != eventIndex) {
    std::ostringstream message;
    message << "g4step_tuple has no event_id=" << eventIndex;
    error = message.str();
    return false;
  }

  if (!data.trackId) {
    error = "g4step_tuple event has no track vectors";
    return false;
  }
  const auto size = data.trackId->size();
  const bool vectorsComplete =
      sameSize(data.parentId, size) && sameSize(data.isPrimary, size) &&
      sameSize(data.trackStepNumber, size) &&
      sameSize(data.recordedStepIndex, size) && sameSize(data.pdg, size) &&
      sameSize(data.stepStatusPre, size) &&
      sameSize(data.processSubtype, size) &&
      sameSize(data.preSensitive, size) && sameSize(data.preX, size) &&
      sameSize(data.preY, size) && sameSize(data.preZ, size) &&
      sameSize(data.postX, size) && sameSize(data.postY, size) &&
      sameSize(data.postZ, size) && sameSize(data.preP, size) &&
      sameSize(data.postP, size) && sameSize(data.loss, size) &&
      sameSize(data.trackLengthPre, size) &&
      sameSize(data.trackLengthPost, size) &&
      sameSize(data.preTouchablePath, size);
  if (!vectorsComplete) {
    error = "g4step_tuple branch-vector sizes disagree";
    return false;
  }

  std::map<int, std::vector<std::size_t>> primaryElectrons;
  for (std::size_t index = 0; index < size; ++index) {
    if (data.parentId->at(index) == 0 && data.isPrimary->at(index) != 0 &&
        std::abs(data.pdg->at(index)) == 11) {
      primaryElectrons[data.trackId->at(index)].push_back(index);
    }
  }
  if (primaryElectrons.size() != 1) {
    std::ostringstream message;
    message << "event " << eventIndex << " contains "
            << primaryElectrons.size()
            << " primary-electron Geant4 tracks; exactly one is required";
    error = message.str();
    return false;
  }

  auto indices = std::move(primaryElectrons.begin()->second);
  std::sort(indices.begin(), indices.end(), [&](std::size_t lhs,
                                                 std::size_t rhs) {
    if (data.trackStepNumber->at(lhs) != data.trackStepNumber->at(rhs))
      return data.trackStepNumber->at(lhs) < data.trackStepNumber->at(rhs);
    return data.recordedStepIndex->at(lhs) <
           data.recordedStepIndex->at(rhs);
  });

  struct Sample {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double p = 0.0;
  };
  struct Anchor {
    double trackLength = 0.0;
    Sample point;
  };

  const auto sampleAt = [&](double trackLength) {
    Sample sample;
    std::size_t selected = indices.back();
    double fraction = 1.0;
    for (const auto index : indices) {
      const double begin = data.trackLengthPre->at(index);
      const double end = data.trackLengthPost->at(index);
      if (trackLength < begin) {
        selected = index;
        fraction = 0.0;
        break;
      }
      if (trackLength <= end || index == indices.back()) {
        selected = index;
        fraction = end > begin
            ? std::clamp((trackLength - begin) / (end - begin), 0.0, 1.0)
            : 1.0;
        break;
      }
    }
    sample.x = data.preX->at(selected) +
        fraction * (data.postX->at(selected) - data.preX->at(selected));
    sample.y = data.preY->at(selected) +
        fraction * (data.postY->at(selected) - data.preY->at(selected));
    sample.z = data.preZ->at(selected) +
        fraction * (data.postZ->at(selected) - data.preZ->at(selected));
    sample.p = data.preP->at(selected) +
        fraction * (data.postP->at(selected) - data.preP->at(selected));
    return sample;
  };

  std::vector<Anchor> anchors;
  for (std::size_t order = 0; order < indices.size(); ++order) {
    const auto index = indices[order];
    // fGeomBoundary is persisted by Geant4 as step-status value 1.
    if (data.preSensitive->at(index) == 0 ||
        data.stepStatusPre->at(index) != 1)
      continue;
    const auto& surface = data.preTouchablePath->at(index);
    if (isTpcUpper(surface)) continue;

    std::size_t endOrder = order;
    while (endOrder + 1 < indices.size() &&
           data.preSensitive->at(indices[endOrder + 1]) != 0 &&
           data.preTouchablePath->at(indices[endOrder + 1]) == surface)
      ++endOrder;
    if (isTpcLower(surface) && endOrder + 1 < indices.size()) {
      const auto& upper =
          data.preTouchablePath->at(indices[endOrder + 1]);
      if (data.preSensitive->at(indices[endOrder + 1]) != 0 &&
          isTpcUpper(upper)) {
        ++endOrder;
        while (endOrder + 1 < indices.size() &&
               data.preSensitive->at(indices[endOrder + 1]) != 0 &&
               data.preTouchablePath->at(indices[endOrder + 1]) == upper)
          ++endOrder;
      }
    }

    const double begin = data.trackLengthPre->at(index);
    const double end = data.trackLengthPost->at(indices[endOrder]);
    if (!(end > begin)) continue;
    const double midpoint = 0.5 * (begin + end);
    anchors.push_back({midpoint, sampleAt(midpoint)});
  }
  if (anchors.size() < 2) {
    std::ostringstream message;
    message << "event " << eventIndex
            << " has fewer than two primary-electron sensitive anchors";
    error = message.str();
    return false;
  }

  intervals.reserve(anchors.size() - 1);
  for (std::size_t interval = 0; interval + 1 < anchors.size(); ++interval) {
    const auto& from = anchors[interval];
    const auto& to = anchors[interval + 1];
    if (!(to.trackLength > from.trackLength) || !(from.point.p > 0.0)) {
      error = "nonphysical sensitive-anchor ordering or momentum";
      intervals.clear();
      return false;
    }
    double ebremLoss = 0.0;
    for (const auto index : indices) {
      const double end = data.trackLengthPost->at(index);
      // Geant4 discrete process subtype 3 is eBrem and belongs to the
      // post-step point.
      if (data.processSubtype->at(index) == 3 &&
          end > from.trackLength && end <= to.trackLength)
        ebremLoss += data.loss->at(index);
    }
    if (!std::isfinite(ebremLoss) || ebremLoss < 0.0 ||
        ebremLoss >= from.point.p) {
      error = "nonphysical Geant4 eBrem interval loss";
      intervals.clear();
      return false;
    }
    intervals.push_back({
        static_cast<int>(interval),
        {{from.point.x, from.point.y, from.point.z}},
        {{to.point.x, to.point.y, to.point.z}},
        from.point.p, ebremLoss});
  }
  return true;
}
