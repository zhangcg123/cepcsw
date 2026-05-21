#ifndef BackgroundEvent_hh
#define BackgroundEvent_hh

#include <map>
#include <vector>
#include <string>
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
    std::map<size_t, edm4hep::SimTrackerHitCollection> oow_tracker_hits; // oow: out-of-window
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
        kMUON = 6, // 100 ns // from Xiaolong Wang
        kNSubDetType
    };

    // different time window cut modes
    // Background hits are always applied the time window cut.
    enum TimeCutMode {
        // kNoCut = 0, // no cut. This option is not used to avoid loading too many hits.
        kCutBkgOnly = 1, // cut only background hits. 
        kCutBoth = 2 // cut both signal and background hits
    };

    std::vector<double> subdet2twindow = {200, 30, 34000, 30, 150, 1000, 100}; // key is subdet, value is time window.
    std::vector<int> subdet2tcutmode = {kCutBoth, kCutBoth, kCutBoth, kCutBoth, kCutBkgOnly, kCutBkgOnly, kCutBoth}; // key is subdet, value is time cut mode.
    std::vector<std::vector<std::string>> subdet2colnames = {
        {"VXD"},
        {"ITK"},
        {"TPC"},
        {"OTK"},
        {"Ecal"},
        {"Hcal"},
        {"Muon"}        
    };

    std::vector<double> subdet2oowtwindow = {1000, 1000, -1, -1, -1, -1, -1}; // key is subdet, value is oow time window. -1 means no oow time window.
};

#endif
