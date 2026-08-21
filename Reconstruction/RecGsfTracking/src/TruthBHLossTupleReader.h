#ifndef RecGsfTracking_TruthBHLossTupleReader_h
#define RecGsfTracking_TruthBHLossTupleReader_h

#include <array>
#include <memory>
#include <string>
#include <vector>

struct TruthBHLossSurfaceInterval {
  int intervalIndex = -1;
  std::array<double, 3> from{{0.0, 0.0, 0.0}};
  std::array<double, 3> to{{0.0, 0.0, 0.0}};
  double momentumBeforeGeV = 0.0;
  double ebremLossGeV = 0.0;
};

/// Lazy reader for the Geant4 pre/post-step tuple written by
/// GsfMaterialStepRecorderAnaElemTool.  It reconstructs the same consecutive
/// sensitive-midpoint intervals as the recorder without loading a whole batch
/// file into memory.
class TruthBHLossTupleReader {
public:
  TruthBHLossTupleReader();
  ~TruthBHLossTupleReader();

  TruthBHLossTupleReader(const TruthBHLossTupleReader&) = delete;
  TruthBHLossTupleReader& operator=(const TruthBHLossTupleReader&) = delete;

  bool open(const std::string& path, std::string& error);
  bool readPrimaryElectronIntervals(
      int eventIndex, std::vector<TruthBHLossSurfaceInterval>& intervals,
      std::string& error);
  long long entries() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif
