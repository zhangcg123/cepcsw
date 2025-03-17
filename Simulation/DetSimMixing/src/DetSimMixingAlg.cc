#include "DetSimMixingAlg.hh"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IRndmEngine.h"

#include <CLHEP/Random/RandExponential.h>
#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandPoisson.h>

#include "BackgroundLoader.hh"

DECLARE_COMPONENT(DetSimMixingAlg)

DetSimMixingAlg::DetSimMixingAlg(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator) {
}

StatusCode DetSimMixingAlg::initialize() {
    StatusCode sc;

    info() << "Initialize DetSimMixingAlg... " << endmsg;
    // preparation according to user properties
    if (m_background_rates.value().size() != m_background_filelists.value().size()) {
        error() << "The size of the background rates and filelists should be the same." << endmsg;
        return StatusCode::FAILURE;
    }

    for (auto [type, rate]: m_background_rates.value()) {
        if (m_background_filelists.value().find(type) == m_background_filelists.value().end()) {
            error() << "The input file lists for the background type " << type << " is not provided." << endmsg;
            return StatusCode::FAILURE;
        }
        m_event_types.push_back(type);
        m_event_rates.push_back(rate);
        m_input_lists.push_back(m_background_filelists.value()[type]);
    }

    // prepare the loaders
    for (size_t i = 0; i < m_event_types.size(); ++i) {
        m_total_rates += m_event_rates[i];
        m_event_loaders.push_back(new BackgroundLoader(m_input_lists[i]));
    }

    if (m_total_rates <= 0) {
        error() << "The total rate of the background events should be positive." << endmsg;
        return StatusCode::FAILURE;
    }

    m_total_tau = 1.0 / m_total_rates;

    info() << "Summary of the background events: " << endmsg;
    for (size_t i = 0; i < m_event_types.size(); ++i) {
        info() << "  Event type: " << m_event_types[i] << ", rate: " << m_event_rates[i] << " Hz" << endmsg;
    }

    return sc;
}

StatusCode DetSimMixingAlg::execute() {
    StatusCode sc;

    info() << "Execute DetSimMixingAlg... " << endmsg;

    // ========================================================================
    // Prepare the batches
    // ========================================================================
    std::vector<BackgroundBatch> batches;

    int nbatches = 5;
    double duration = 1e3; // 1 us = 1,000 ns of each batch
    TTimeStamp start_time;

    for (int i = 0; i < nbatches; i++) {
        BackgroundBatch batch;
        batch.start_time = start_time + TTimeStamp(0, static_cast<int>(i*duration));
        batch.duration = duration;

        // todo: need to sample according to the rate
        batch.num_events = 0;
        for (size_t evttype = 0; evttype < m_event_types.size(); ++evttype) {
            batch.event_numbers_groupby_type.push_back(0);
        }

        double current_time = 0;

        while (true) {
            // sampling and get an event
            size_t selected_evttype = 0;
            double r = CLHEP::RandFlat::shoot(m_total_rates);
            double accumulated = 0;
            for (size_t evttype = 0; evttype < m_event_types.size(); ++evttype) {
                accumulated += m_event_rates[evttype];
                if (r < accumulated) {
                    selected_evttype = evttype;
                    break;
                }
            }

            // sampling the time
            current_time += CLHEP::RandExponential::shoot(m_total_tau)*1e9; // ns

            if (current_time > duration) {
                break;
            }

            // add the event to Batch.
            ++batch.num_events;
            batch.event_types.push_back(selected_evttype);
            batch.event_times.push_back(current_time);

            batch.event_numbers_groupby_type[selected_evttype] += 1;
        }

        batches.push_back(batch);
    }


    // ========================================================================
    // Unpack the hits from Loader and fill the existing hits into collections
    // of BackgroundEvent. 
    // ========================================================================
    BackgroundEvent bkg_evt;

    for (size_t i = 0; i < batches.size(); ++i) {
        const BackgroundBatch& batch = batches[i];

        for (size_t j = 0; j < batch.num_events; ++j) {
            size_t evttype = batch.event_types[j];
            double evttime = batch.event_times[j];

            IBackgroundLoader* loader = m_event_loaders[evttype];

            if (not loader->fill(bkg_evt, evttime)) {
                error() << "Failed to load the next event." << endmsg;
                return StatusCode::FAILURE;
            }
            
        }
    }

    // dump the information
    for (auto [name, idx]: bkg_evt.collection_index) {
        info() << "Collection: " << name << ", index: " << idx;
        if (bkg_evt.tracker_hits.count(idx)) {
            info() << "  Tracker hits: " << bkg_evt.tracker_hits[idx].size() << endmsg;
        } else if (bkg_evt.calorimeter_hits.count(idx)) {
            info() << "  Calorimeter hits: " << bkg_evt.calorimeter_hits[idx].size() << endmsg;
        } else {
            info() << endmsg;
        }
    }

    // ========================================================================
    // Put the BackgroundEvent into the event store
    // ========================================================================

    return sc;
}

StatusCode DetSimMixingAlg::finalize() {
    StatusCode sc;

    info() << "Finalize DetSimMixingAlg... " << endmsg;

    return sc;
}