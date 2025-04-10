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
    std::map<std::string, size_t> collection_subdet; // key is collection name, value is subdetector type.

    // in order to reduce the memory usage, need additional filter setup here.
    // for example, for TPC, we need to load all the hits.
    // however, for the tracker/calo, we only need to load the hits which are
    // in an interested event time window.
    enum SubDetType {
        kVXD = 0,  // 200 ns
        kITK = 1,  // 200 ns
        kTPC = 2,  // 34 us
        kOTK = 3,  // 1 us
        kECAL = 4, // 150 ns
        kHCAL = 5, // 1 us
        kMUON = 6, // 1 us // todo
        kNSubDetType
    };
    std::vector<double> subdet2twindow = {200, 200, 34000, 1000, 150, 1000, 1000}; // key is subdet, value is time window.
    std::vector<std::vector<std::string>> subdet2colnames = {
        {"VXD"},
        {"ITK"},
        {"TPC"},
        {"OTK"},
        {"Ecal"},
        {"Hcal"},
        {"Muon"}        
    };
};

class IBackgroundLoader {
public:
    virtual ~IBackgroundLoader() = default;

    // the loader is responsible to unpack the hits into background event.
    // if failed, then return false.
    virtual bool fill(BackgroundEvent& bkg_event, const double current_time_in_ns) = 0;
};

#endif