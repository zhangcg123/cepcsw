#ifndef BackgroundLoader_hh
#define BackgroundLoader_hh

#include "IBackgroundLoader.hh"
#include <iostream>
#include <vector>
#include <string>

#include <podio/Frame.h>
#include <podio/ROOTFrameReader.h>

class BackgroundLoader: public IBackgroundLoader {
public:

    BackgroundLoader(const std::vector<std::string>& filenames) {
        m_reader.openFiles(filenames);
        m_max_evt = m_reader.getEntries(podio::Category::Event);
    }
    virtual ~BackgroundLoader() = default;

    bool fill(BackgroundEvent& evt, const double current_time_in_ns) override {
        // before load the next event, check whether end or not.
        // if reach the end, then go back to the first event.
        if (m_current_evt >= m_max_evt) {
            m_current_evt = 0;
        }

        // get the frame of the current event
        auto frame = podio::Frame(m_reader.readEntry(podio::Category::Event, m_current_evt));
        
        // before unpack the hits, check the collections name to index map.
        if (evt.collection_index.empty()) {
            size_t idx = 0;
            for (auto col_name: frame.getAvailableCollections()) {
                // std::cout << "available collection: " << col_name << std::endl;
                evt.collection_index[col_name] = idx;
                ++idx;

                // find the corresponding subdetector type.
                for (size_t subdet = 0; subdet < BackgroundEvent::kNSubDetType; ++subdet) {
                    for (auto key: evt.subdet2colnames[subdet]) {
                        if (col_name.find(key) != std::string::npos) {
                            evt.collection_subdet[col_name] = subdet;
                            break;
                        }
                    }
                }
            }
        }

        for (auto [name, colidx]: evt.collection_index) {
            auto col_ = frame.get(name);

            // check whether the collection has a valid subdetector type.
            if (evt.collection_subdet.find(name) == evt.collection_subdet.end()) {
                continue;
            }

            auto time_window = evt.subdet2twindow[evt.collection_subdet[name]];
            
            // if the current time is out of the time window, then skip this collection.
            if (current_time_in_ns < -time_window || current_time_in_ns > time_window) {
                continue;
            }

            std::cout << "Collection: " << name 
                      << ", index: " << colidx 
                      << ", time window: " << time_window << " ns"
                      << ", current time: " << current_time_in_ns;

            // debug only. don't create any hits.
            // conclusion: no memory leakage in the above code.
            // continue;

            int counter = 0;

            if (auto col = dynamic_cast<const edm4hep::SimTrackerHitCollection*>(col_)) {
                auto& trk_col = evt.tracker_hits[colidx];
                for (auto oldhit: *col) {
                    auto t = oldhit.getTime() + current_time_in_ns;
                    if (t < -time_window || t > time_window) {
                        // if the hit is not in the time window, skip.
                        continue;
                    }

                    auto newhit = trk_col.create();
                    newhit.setCellID(oldhit.getCellID());
                    newhit.setEDep(oldhit.getEDep());
                    newhit.setTime(t); // new
                    newhit.setPathLength(oldhit.getPathLength());
                    newhit.setQuality(oldhit.getQuality());
                    newhit.setPosition(oldhit.getPosition());
                    newhit.setMomentum(oldhit.getMomentum());

                    // extra
                    newhit.setOverlay(true);

                    ++counter;
                }
                
            } else if (auto col = dynamic_cast<const edm4hep::SimCalorimeterHitCollection*>(col_)) {
                auto& calo_col = evt.calorimeter_hits[colidx];
                auto& calo_contrib_col = evt.calo_contribs[colidx];
                for (auto oldhit: *col) {
                    // check whether the hit is in the time window.
                    bool is_in_window = false;
                    for (auto contrib: oldhit.getContributions()) {
                        auto t = contrib.getTime() + current_time_in_ns;
                        if (t < -time_window || t > time_window) {
                            continue;
                        }
                        is_in_window = true;
                        break;
                    }
                    if (not is_in_window) {
                        continue;
                    }

                    auto newhit = calo_col.create();
                    newhit.setCellID(oldhit.getCellID());
                    newhit.setEnergy(oldhit.getEnergy());
                    newhit.setPosition(oldhit.getPosition());
                    // todo: fixme
                    // loop all the contributions and add the time.
                    for (auto contrib: oldhit.getContributions()) {
                        auto newcontrib = calo_contrib_col.create();
                        newcontrib.setPDG(contrib.getPDG());
                        newcontrib.setEnergy(contrib.getEnergy());
                        newcontrib.setStepPosition(contrib.getStepPosition());
                        newcontrib.setTime(contrib.getTime() + current_time_in_ns);
                        newhit.addToContributions(newcontrib);
                    }
                    ++counter;
                }
            } else {
                continue;
            }

            std::cout << ", counter: " << counter << std::endl;

        }

        ++m_current_evt;
        return true;
    }

private:
    podio::ROOTFrameReader m_reader;

    unsigned int m_current_evt{0};
    unsigned int m_max_evt{0};

};


#endif