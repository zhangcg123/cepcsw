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

#include "BackgroundBatch.hh"
#include "IBackgroundLoader.hh"

class DetSimMixingAlg: public Algorithm {
public:
    DetSimMixingAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

private:
    double m_total_rates{0}; // Hz. 
    double m_total_tau{0}; // s. the total time of the background events

private:
    std::vector<std::string> m_event_types; // the event types of the background events
    std::vector<double> m_event_rates; // the rates of the background events
    std::vector<IBackgroundLoader*> m_event_loaders; // the loaders of the background events
    std::vector<std::vector<std::string>> m_input_lists; // input files for each backgroud

private:
    // properties for user side
    Gaudi::Property<std::map<std::string, double>> m_background_rates{this, "BackgroundRates", {}, "The rates of the background events"};
    Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_background_filelists{this, "BackgroundFileLists", {}, "The input file lists for the background events"};
};

#endif
