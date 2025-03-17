#ifndef BackgroundBatch_hh
#define BackgroundBatch_hh

#include <cstdint>
#include <vector>
#include "TTimeStamp.h"

class BackgroundBatch {
public:
    TTimeStamp start_time; // the start time of the batch
    double duration; // the duration of the batch (in ns)
    int num_events; // the number of events in the batch

    // below are the metadata of each type of backgrouds in the batch
    std::vector<int> event_numbers_groupby_type; // the number of events for each type

    // below are the metadata of each event in the batch
    std::vector<size_t> event_types; // the event types in the batch
    std::vector<double> event_times; // the relative event times in the batch (in ns)
};

#endif
