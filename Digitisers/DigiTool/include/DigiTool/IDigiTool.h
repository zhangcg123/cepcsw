#ifndef IDigiTool_h
#define IDigiTool_h

/*
 * Description:
 *   IDigiTool is used to perform digitizer
 *
 * The interface:
 *   * Call: peform on digitizer
 *
 */

#include "GaudiKernel/IAlgTool.h"

namespace edm4hep{
  class SimTrackerHit;
  class SimTrackerHitCollection;
  class TrackerHitCollection;
  class MCRecoTrackerAssociationCollection;
}

class IDigiTool: virtual public IAlgTool {
 public:

  DeclareInterfaceID(IDigiTool, 0, 1);
  virtual ~IDigiTool() {}

  virtual StatusCode Call(const edm4hep::SimTrackerHitCollection* simCol, edm4hep::TrackerHitCollection* hitCol,
			  edm4hep::MCRecoTrackerAssociationCollection* assCol) = 0;
  virtual StatusCode Call(edm4hep::SimTrackerHit simhit, edm4hep::TrackerHitCollection* hitCol,
			  edm4hep::MCRecoTrackerAssociationCollection* assCol) = 0;
};
#endif
