#ifndef GSFTRUTHEVENTDATA_COMPACTTRUTH_H
#define GSFTRUTHEVENTDATA_COMPACTTRUTH_H
#include <cmath>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>
namespace gsftruth {
struct StepSpan {
  int trackID=-1, first=-1, last=-1, count=0;
  double tx0=0;
};
struct CompactSteps {
  std::vector<std::size_t> kept;
  std::vector<StepSpan> spans;
};
// Only non-radiative interior steps may be omitted. Gaps remain gaps.
template<class Step>
CompactSteps compactSteps(const std::vector<Step>& steps,
                         const std::set<std::pair<int,int>>& checkpoints) {
  CompactSteps result;
  StepSpan span;
  auto flush=[&] { if (span.count) result.spans.push_back(span); span=StepSpan{}; };
  for (std::size_t i=0;i<steps.size();++i) {
    const auto& s=steps[i];
    if (i==0 || i+1==steps.size() || steps[i-1].trackID!=s.trackID || steps[i+1].trackID!=s.trackID ||
        s.processSubtype==3 || s.firstEcalEntry || s.ecalEntryOnly ||
        checkpoints.count({s.trackID,s.stepNumber}) || !std::isfinite(s.stepTX0) || s.stepTX0<0) {
      flush(); result.kept.push_back(i); continue;
    }
    if (span.count && (span.trackID!=s.trackID || span.last+1!=s.stepNumber)) flush();
    if (!span.count) { span.trackID=s.trackID; span.first=s.stepNumber; }
    span.last=s.stepNumber; ++span.count; span.tx0+=s.stepTX0;
  }
  flush(); return result;
}
}
#endif
