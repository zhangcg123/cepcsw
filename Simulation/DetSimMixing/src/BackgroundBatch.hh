#ifndef BackgroundBatch_hh
#define BackgroundBatch_hh

#include <cstdint>
#include <vector>
#include "TTimeStamp.h"

struct BackgroundBatch {
    double start_time; // ns, the start time of the batch, relative to signal event
    double duration; // the duration of the batch
    int num_events; // the number of events in the batch

    // below are the metadata of each type of backgrouds in the batch
    std::vector<int> event_numbers_groupby_type; // the number of events for each type

    // below are the metadata of each event in the batch
    std::vector<size_t> event_types; // the event types in the batch
    std::vector<double> event_times; // the relative event times in the batch (in ns)
};

#endif
