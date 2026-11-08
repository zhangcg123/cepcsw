#ifndef GSFTRUTHEVENTDATA_GENERATORFSR_H
#define GSFTRUTHEVENTDATA_GENERATORFSR_H
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

namespace gsftruth {
struct GeneratorFsrTag {
  int photon = -1, emitter = -1, electron = -1, status = 1;
};

// Stop at non-electron production nodes: incoming beams are not outgoing FSR ancestors.
inline std::vector<GeneratorFsrTag> generatorFsr(
    const std::vector<int>& pdg, const std::vector<int>& status,
    const std::vector<std::vector<int>>& parents) {
  if (pdg.size()!=status.size() || pdg.size()!=parents.size())
    throw std::invalid_argument("Misaligned generator history");
  for (const auto& row : parents) for (int p : row)
    if (p<0 || static_cast<std::size_t>(p)>=pdg.size())
      throw std::invalid_argument("Unavailable generator parent");
  std::map<int,std::set<int>> owners;
  for (std::size_t final=0; final<pdg.size(); ++final) {
    if (status[final]!=1 || (pdg[final]!=11 && pdg[final]!=-11)) continue;
    std::set<int> seen, active;
    std::function<void(int)> visit=[&](int i) {
      if (active.count(i)) throw std::invalid_argument("Cyclic generator electron history");
      if (!seen.insert(i).second) return;
      active.insert(i); owners[i].insert(static_cast<int>(final));
      for (int p : parents[i]) if (pdg[p]==pdg[final]) visit(p);
      active.erase(i);
    };
    visit(static_cast<int>(final));
  }
  std::vector<GeneratorFsrTag> result;
  for (std::size_t i=0;i<pdg.size();++i) {
    if (pdg[i]!=22 || status[i]!=1) continue;
    std::set<int> electrons, emitters;
    for (int p : parents[i]) {
      const auto found=owners.find(p);
      if (found==owners.end()) continue;
      emitters.insert(p); electrons.insert(found->second.begin(),found->second.end());
    }
    if (electrons.empty()) continue;
    GeneratorFsrTag tag; tag.photon=i;
    tag.status=electrons.size()==1 && emitters.size()==1 ? 1 : 2;
    if (electrons.size()==1) tag.electron=*electrons.begin();
    if (emitters.size()==1) tag.emitter=*emitters.begin();
    result.push_back(tag);
  }
  return result;
}
}
#endif
