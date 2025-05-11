#ifndef DetSimMixingAlg_hh
#define DetSimMixingAlg_hh
/*
 * Description:
 *   This algorithm is used to generate a mixing event. 
 *   Assume there will be one physics event and multiple background events.
 * 
 *   The main algorithm first creates enough BackgroundBatch instances. 
 *   All the metadata will be prepared into the batch.
 *   Then, the main algorithm will start the mixing and load the real hits.
 *   The reason to unpack the hits in main algorithm is to reduce the data copy.
 * 
 *   About the BackgroundBatch:
 * 
 *     -----------------------------------------------> time
 *        |  o   x      o             o       x   |
 *        |  |   |
 *        |  |    \-> group by event type 'x', num of events = 2
 *        |   -> group by event type 'o', num of events = 3
 *         \-> start time
 * 
 *   For each event, it consists of several collections. For one batch:
 * 
 *     | EventType | Collection 1 | Collection 2 | ... | Collection N |
 *     |     o     |              |              | ... |              |
 *     |     x     |              |              | ... |              |
 *     |     o     |              |              | ... |              |
 *     |     o     |              |              | ... |              |
 *     |     x     |              |              | ... |              |
 * 
 *   The main algorithm needs to loop BackgroundLoader and unpack the collections.
 *   For each iteration in the loop of the above table, a current event could be accessed. 
 *   According to the current event, the main algorithm could unpack the collections.
 * 
 * Authors:
 *   Tao Lin <lintao AT ihep.ac.cn>
 */

#include <string>
#include <vector>
#include <map>

#include <GaudiKernel/Algorithm.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/ToolHandle.h>

#include "k4FWCore/DataHandle.h"

#include "IBackgroundLoader.hh"
#include "BackgroundBatch.hh"
#include "BackgroundEvent.hh"

class DetSimMixingAlg: public Algorithm {
public:
    DetSimMixingAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

private:
    double m_total_rates{0}; // Hz. 
    double m_total_tau{0}; // s. 

private:
    std::vector<std::string> m_event_types; // the event types of the background events
    std::vector<double> m_event_timings; // the rates or durations of the background events
    std::vector<IBackgroundLoader*> m_event_loaders; // the loaders of the background events
    std::vector<std::vector<std::string>> m_input_lists; // input files for each backgroud

    // following maps are used to load signal collections from memory
    std::map<std::string, DataHandle<edm4hep::SimTrackerHitCollection>*> m_sig_trackerColMap;
    std::map<std::string, DataHandle<edm4hep::SimCalorimeterHitCollection>*> m_sig_calorimeterColMap;
    std::map<std::string, DataHandle<edm4hep::CaloHitContributionCollection>*> m_sig_caloContribColMap;

private:
    // following maps are used to put collections into memory
    std::map<std::string, DataHandle<edm4hep::SimTrackerHitCollection>*> m_trackerColMap;
    std::map<std::string, DataHandle<edm4hep::SimCalorimeterHitCollection>*> m_calorimeterColMap;
    std::map<std::string, DataHandle<edm4hep::CaloHitContributionCollection>*> m_caloContribColMap;

    // the name here is without suffix "Collection"
    Gaudi::Property<std::vector<std::string>> m_trackerColNames{this, 
        "TrackerCollections",
	{"VXD", "ITKBarrel", "ITKEndcap", "TPC", "TPCLowPt", "TPCSpacePoint",
	 "OTKBarrel", "OTKEndcap", "MuonBarrel", "MuonEndcap"},
        "Names of the Tracker collections (without suffix Collection)"};
    Gaudi::Property<std::vector<std::string>> m_calorimeterColNames{this, 
        "CalorimeterCollections",
        {"EcalBarrel", "EcalEndcaps", "EcalEndcapRing", 
         "HcalBarrel", "HcalEndcaps", "HcalEndcapRing"}, 
        "Names of the Calorimeter collections (without suffix Collection)"};

private:
    // Following properties are used to configure different types of background events.
    // * Key: label of one type
    // * Value: 
    //   * timing: 
    //     * FixedTimeWindow: if the rate is less than 0, then always mix the background events. fabs(rate) is the time window of the background events.
    //     * Sampled: if the rate is larger than 0, then the background events will be sampled according to the rate.
    //   * filelist: the input file list for this type of background events.
    Gaudi::Property<std::map<std::string, double>> m_background_timings{this, "BackgroundTimings", {}, "The rates (positive, Hz) or time windows (negative, ns) of the background events"};
    Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_background_filelists{this, "BackgroundFileLists", {}, "The input file lists for the background events"};


    // Bunch Crossing information
    Gaudi::Property<double> m_bunch_crossing_spacing{this, "BunchCrossingSpacing", 277.0, "The bunch crossing spacing in ns"};
    Gaudi::Property<int> m_nbunches_per_batch{this, "NbunchesPerBatch", 10, "The number of bunches per batch"};
    Gaudi::Property<int> m_nbatches{this, "Nbatches", 13, "The number of batches to be mixed"}; // 34us/(277ns*10) = ~13

private:
    // Time window for VXD, ITK, TPC, OTK, ECAL, HCAL, MUON
    // Only the hits between [-T, T] ns are loaded
    Gaudi::Property<double> m_vxd_time_window{this, "VXDTimeWindow", 200.0, "The time window for VXD in ns"};
    Gaudi::Property<double> m_itk_time_window{this, "ITKTimeWindow", 30.0, "The time window for ITK in ns"};
    Gaudi::Property<double> m_tpc_time_window{this, "TPCTimeWindow", 34000.0, "The time window for TPC in ns"};
    Gaudi::Property<double> m_otk_time_window{this, "OTKTimeWindow", 30.0, "The time window for OTK in ns"};
    Gaudi::Property<double> m_ecal_time_window{this, "EcalTimeWindow", 150.0, "The time window for ECAL in ns"};
    Gaudi::Property<double> m_hcal_time_window{this, "HcalTimeWindow", 1000.0, "The time window for HCAL in ns"};
    Gaudi::Property<double> m_muon_time_window{this, "MuonTimeWindow", 100.0, "The time window for MUON in ns"};
};

#endif
