#ifndef IBackgroundLoader_hh
#define IBackgroundLoader_hh

#include "BackgroundBatch.hh"
#include <map>

#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/CaloHitContributionCollection.h"

/*
 * Description:
 *   As there are multiple collections for one event, this BackgrounEvent
 *   is used to organize these collections.
 * 
 *   The key is the index of collection. 
 * 
 *   BackgroundLoader is resposible to fill its hit collections into the
 *   BackgroundEvent. 
 */
struct BackgroundEvent {
    std::map<size_t, edm4hep::SimTrackerHitCollection> tracker_hits;
    std::map<size_t, edm4hep::SimCalorimeterHitCollection> calorimeter_hits;
    std::map<size_t, edm4hep::CaloHitContributionCollection> calo_contribs;

    std::map<std::string, size_t> collection_index; // key is collection name, value is index.
};

class IBackgroundLoader {
public:
    virtual ~IBackgroundLoader() = default;

    // the loader is responsible to unpack the hits into background event.
    // if failed, then return false.
    virtual bool fill(BackgroundEvent& bkg_event, const double current_time_in_ns) = 0;
};

#endif