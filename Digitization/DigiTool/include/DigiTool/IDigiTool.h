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
#include "edm4hep/EDM4hepVersion.h"

#include "edm4hep/SimTrackerHitCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif


class IDigiTool: virtual public IAlgTool {
 public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif

  DeclareInterfaceID(IDigiTool, 0, 1);
  virtual ~IDigiTool() {}

  virtual StatusCode Call(const edm4hep::SimTrackerHitCollection* simCol, CEPCSWTrackerHit3DCollection* hitCol,
			  CEPCSWTrackerHitSimTrackerHitLinkCollection* assCol) = 0;
  virtual StatusCode Call(edm4hep::SimTrackerHit simhit, CEPCSWTrackerHit3DCollection* hitCol,
			  CEPCSWTrackerHitSimTrackerHitLinkCollection* assCol) = 0;
};
#endif
