#include "DetSimMixingAlg.hh"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IRndmEngine.h"

#include <CLHEP/Random/RandExponential.h>
#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandPoisson.h>

#include "BackgroundLoader.hh"
#include "BackgroundEvent.hh"

DECLARE_COMPONENT(DetSimMixingAlg)

DetSimMixingAlg::DetSimMixingAlg(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator) {
}

StatusCode DetSimMixingAlg::initialize() {
    StatusCode sc;

    info() << "Initialize DetSimMixingAlg... " << endmsg;
    // preparation according to user properties
    if (m_background_timings.value().size() != m_background_filelists.value().size()) {
        error() << "The size of the background rates and filelists should be the same." << endmsg;
        return StatusCode::FAILURE;
    }

    for (auto [type, timing]: m_background_timings.value()) {
        if (m_background_filelists.value().find(type) == m_background_filelists.value().end()) {
            error() << "The input file lists for the background type " << type << " is not provided." << endmsg;
            return StatusCode::FAILURE;
        }
        m_event_types.push_back(type);
        m_event_timings.push_back(timing);
        m_input_lists.push_back(m_background_filelists.value()[type]);
    }

    // prepare the loaders
    for (size_t i = 0; i < m_event_types.size(); ++i) {
        // only the positive rates are considered into total rates
        if (m_event_timings[i] > 0) {
            m_total_rates += m_event_timings[i];
        }
        m_event_loaders.push_back(new BackgroundLoader(m_input_lists[i]));
    }

    if (m_total_rates < 0) {
        error() << "The total rate of the background events should be >= 0." << endmsg;
        return StatusCode::FAILURE;
    } else if (m_total_rates == 0) {
        info() << "All background events will be loaded in fixed time window mode." << endmsg;
    } else {
        m_total_tau = 1.0 / m_total_rates;
    }
    info() << "Summary of the background events: " << endmsg;
    for (size_t i = 0; i < m_event_types.size(); ++i) {
        if (m_event_timings[i] > 0) {
            info() << "  Event type: " << m_event_types[i] << ", rate: " << m_event_timings[i] << " Hz" << endmsg;
        } else {
            info() << "  Event type: " << m_event_types[i] << ", time window: " << fabs(m_event_timings[i]) << " ns" << endmsg;
        }
    }

    // prepare for the output
    // for simplicity, the collection names are with prefix. the key in the map is without prefix, so it is same to the original input.
    std::string prefix = "Mixed";
    // SimTrackerHitCollections
    for (const auto& name : m_trackerColNames) {
        std::string name_col = name + "Collection";
        auto col = new DataHandle<edm4hep::SimTrackerHitCollection>(prefix + name_col, Gaudi::DataHandle::Writer, this);
        m_trackerColMap[name_col] = col;

        auto sig_col = new DataHandle<edm4hep::SimTrackerHitCollection>(name_col, Gaudi::DataHandle::Reader, this);
        m_sig_trackerColMap[name_col] = sig_col;
        
    }

    // SimCalorimeterHitCollections
    for (const auto& name : m_calorimeterColNames) {
        std::string name_col = name + "Collection";
        std::string name_contrib_col = name + "ContributionCollection";
        auto col = new DataHandle<edm4hep::SimCalorimeterHitCollection>(prefix + name_col, Gaudi::DataHandle::Writer, this);
        m_calorimeterColMap[name_col] = col;
        auto col_contrib = new DataHandle<edm4hep::CaloHitContributionCollection>(prefix + name_contrib_col, Gaudi::DataHandle::Writer, this);
        m_caloContribColMap[name_col] = col_contrib;

        auto sig_col = new DataHandle<edm4hep::SimCalorimeterHitCollection>(name_col, Gaudi::DataHandle::Reader, this);
        m_sig_calorimeterColMap[name_col] = sig_col;
        auto sig_col_contrib = new DataHandle<edm4hep::CaloHitContributionCollection>(name_contrib_col, Gaudi::DataHandle::Reader, this);
        m_sig_caloContribColMap[name_col] = sig_col_contrib;

    }

    return sc;
}

StatusCode DetSimMixingAlg::execute() {
    StatusCode sc;

    info() << "Execute DetSimMixingAlg... " << endmsg;

    // ========================================================================
    // Reset
    // ========================================================================
    double start_time{0.0}; // ns, the start time of the event, always align to the signal

    // ========================================================================
    // Prepare the batches
    // ========================================================================
    std::vector<BackgroundBatch> batches;

    int nbatches = m_nbatches.value(); // the total number of batches will be 2*nbatches, [-nbatches, nbatches)
    int nbunches = m_nbunches_per_batch.value(); // the total number of bunches per batch
    double duration = nbunches * m_bunch_crossing_spacing.value(); // duration of each batch (ns): Nbunch x TBunchSpacing = 10 x 277 ns = 2770 ns

    for (int i = -nbatches; i < nbatches; ++i) {
        BackgroundBatch batch;
        batch.start_time = start_time + i*duration;
        batch.duration = duration;

        // todo: need to sample according to the rate
        batch.num_events = 0;
        for (size_t evttype = 0; evttype < m_event_types.size(); ++evttype) {
            batch.event_numbers_groupby_type.push_back(0);
        }

        double current_time = 0;

        // =======================================================================
        // fixed time window mode
        // =======================================================================
        // insert at beginning, align to the begin of batch
        for (size_t evttype = 0; evttype < m_event_types.size(); ++evttype) {
            // skip the sampled mode type
            if (m_event_timings[evttype] > 0) {
                continue;
            }
            double time_window_event = fabs(m_event_timings[evttype]);
            int n_events = std::max(1, static_cast<int>(batch.duration/time_window_event)); // ns

            for (size_t j = 0; j < n_events; ++j) {
                double this_current_time = j * time_window_event; // ns
                ++batch.num_events;
                batch.event_types.push_back(evttype);
                batch.event_times.push_back(this_current_time);
                batch.event_numbers_groupby_type[evttype] += 1;
            }
        }

        // =======================================================================
        // sampled mode
        // =======================================================================
        while (m_total_tau>0) {
            // sampling and get an event
            size_t selected_evttype = 0;
            double r = CLHEP::RandFlat::shoot(m_total_rates);
            double accumulated = 0;
            for (size_t evttype = 0; evttype < m_event_types.size(); ++evttype) {
                // skip the fixed time mode type
                if (m_event_timings[evttype] <= 0) {
                    continue;
                }
                accumulated += m_event_timings[evttype];
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
    // setup the time window
    bkg_evt.subdet2twindow[BackgroundEvent::kVXD] = m_vxd_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kITK] = m_itk_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kTPC] = m_tpc_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kOTK] = m_otk_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kECAL] = m_ecal_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kHCAL] = m_hcal_time_window.value();
    bkg_evt.subdet2twindow[BackgroundEvent::kMUON] = m_muon_time_window.value();

    info() << "Creating a BackgroundEvent..." << endmsg;
    for (size_t i = 0; i < batches.size(); ++i) {
        info() << "  Batch " << i << ": " << endmsg;
        const BackgroundBatch& batch = batches[i];

        for (size_t j = 0; j < batch.num_events; ++j) {
            size_t evttype = batch.event_types[j];
            double evttime = batch.start_time + batch.event_times[j];
            info() << "    Event type: " << m_event_types[evttype] << ", time: " << evttime << " ns" << endmsg;

            IBackgroundLoader* loader = m_event_loaders[evttype];

            if (not loader->fill(bkg_evt, evttime)) {
                error() << "Failed to load the next event." << endmsg;
                return StatusCode::FAILURE;
            }
            
        }
    }
    info() << "BackgroundEvent created." << endmsg;
    info() << "Summary: " << endmsg;
    // dump the information
    for (auto [name, idx]: bkg_evt.collection_index) {
        info() << "- Collection: " << name << ", index: " << idx;
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
    // add prefix 'Mixed'.

    // Need to prepare a complete list of collection names.
    std::map<std::string, size_t> col_names;
    for (auto [name, idx]: bkg_evt.collection_index) {
        ++col_names[name];
    }
    for (auto [name, idx]: m_sig_trackerColMap) {
        if (col_names.find(name) == col_names.end()) {
            warning() << "Collection " << name << " is not in the background event." << endmsg;
        }
        ++col_names[name];
    }
    for (auto [name, idx]: m_sig_calorimeterColMap) {
        if (col_names.find(name) == col_names.end()) {
            warning() << "Collection " << name << " is not in the background event." << endmsg;
        }
        ++col_names[name];
    }

    for (auto [col_name, cnt]: col_names) {

        if (bkg_evt.collection_index.find(col_name) == bkg_evt.collection_index.end()) {
            debug() << "Collection " << col_name << " is not in the background event." << endmsg;
            continue;
        }
        
        auto colidx = bkg_evt.collection_index[col_name];

        if (bkg_evt.tracker_hits.count(colidx)) {
            auto& col = bkg_evt.tracker_hits[colidx];
            
            auto newcol = m_trackerColMap[col_name]->createAndPut();

            // background
            for (auto oldhit: col) {
                auto newhit = newcol->create();
                newhit.setCellID(oldhit.getCellID());
                newhit.setEDep(oldhit.getEDep());
                newhit.setTime(oldhit.getTime()); // new
                newhit.setPathLength(oldhit.getPathLength());
                newhit.setQuality(oldhit.getQuality());
                newhit.setPosition(oldhit.getPosition());
                newhit.setMomentum(oldhit.getMomentum());
            }

            // signal
            auto sig_col = m_sig_trackerColMap[col_name]->get();
            if (!sig_col) {
                continue;
            }

            // keep the same time windows for signal and backgrounds.
            auto time_window = bkg_evt.subdet2twindow[bkg_evt.collection_subdet[col_name]];

            for (auto oldhit: *sig_col) {
                auto t = oldhit.getTime();
                if (t < -time_window || t > time_window) {
                    // if the hit is not in the time window, skip.
                    continue;
                }

                auto newhit = newcol->create();
                newhit.setCellID(oldhit.getCellID());
                newhit.setEDep(oldhit.getEDep());
                newhit.setTime(oldhit.getTime()); // new
                newhit.setPathLength(oldhit.getPathLength());
                newhit.setQuality(oldhit.getQuality());
                newhit.setPosition(oldhit.getPosition());
                newhit.setMomentum(oldhit.getMomentum());

                // associate MC particle for signal
                newhit.setMCParticle(oldhit.getMCParticle());
            }

        } else if (bkg_evt.calorimeter_hits.count(colidx)) {
            auto& col = bkg_evt.calorimeter_hits[colidx];
            auto& col_contrib = bkg_evt.calo_contribs[colidx];
            
            auto newcol = m_calorimeterColMap[col_name]->createAndPut();
            auto newcol_contrib = m_caloContribColMap[col_name]->createAndPut();

            for (auto oldhit: col) {
                auto newhit = newcol->create();
                newhit.setCellID(oldhit.getCellID());
                newhit.setEnergy(oldhit.getEnergy());
                newhit.setPosition(oldhit.getPosition());
                // todo: fixme
                // loop all the contributions and add the time.
                for (auto contrib: oldhit.getContributions()) {
                    auto newcontrib = newcol_contrib->create();
                    newcontrib.setPDG(contrib.getPDG());
                    newcontrib.setEnergy(contrib.getEnergy());
                    newcontrib.setStepPosition(contrib.getStepPosition());
                    newcontrib.setTime(contrib.getTime());
                    newhit.addToContributions(newcontrib);
                }
            }

            // signal
            auto sig_col = m_sig_calorimeterColMap[col_name]->get();
            auto sig_col_contrib = m_sig_caloContribColMap[col_name]->get();

            if (!sig_col || !sig_col_contrib) {
                continue;
            }

            // keep the same time windows for signal and backgrounds.
            auto time_window = bkg_evt.subdet2twindow[bkg_evt.collection_subdet[col_name]];


            for (auto oldhit: *sig_col) {
                // check whether the hit is in the time window.
                bool is_in_window = false;
                for (auto contrib: oldhit.getContributions()) {
                    auto t = contrib.getTime();
                    if (t < -time_window || t > time_window) {
                        continue;
                    }
                    is_in_window = true;
                    break;
                }
                if (not is_in_window) {
                    continue;
                }

                auto newhit = newcol->create();
                newhit.setCellID(oldhit.getCellID());
                newhit.setEnergy(oldhit.getEnergy());
                newhit.setPosition(oldhit.getPosition());
                // todo: fixme
                // loop all the contributions and add the time.
                for (auto contrib: oldhit.getContributions()) {
                    auto newcontrib = newcol_contrib->create();
                    newcontrib.setPDG(contrib.getPDG());
                    newcontrib.setEnergy(contrib.getEnergy());
                    newcontrib.setStepPosition(contrib.getStepPosition());
                    newcontrib.setTime(contrib.getTime());
                    newhit.addToContributions(newcontrib);
                }
            }
            
        } else {
            debug() << "The collection " << col_name 
                    << " with idx" << colidx 
                    << " is not found." << endmsg;
            continue;
        }
    }


    return sc;
}

StatusCode DetSimMixingAlg::finalize() {
    StatusCode sc;

    info() << "Finalize DetSimMixingAlg... " << endmsg;

    return sc;
}
