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
            }
        }

        for (auto [name, colidx]: evt.collection_index) {
            auto col_ = frame.get(name);

            if (auto col = dynamic_cast<const edm4hep::SimTrackerHitCollection*>(col_)) {
                auto& trk_col = evt.tracker_hits[colidx];
                for (auto oldhit: *col) {
                    auto newhit = oldhit.clone();
                    newhit.setTime(oldhit.getTime() + current_time_in_ns);
                    newhit.setOverlay(true);
                    trk_col.push_back(newhit);
                }
                
            } else if (auto col = dynamic_cast<const edm4hep::SimCalorimeterHitCollection*>(col_)) {
                auto& calo_col = evt.calorimeter_hits[colidx];
                for (auto oldhit: *col) {
                    auto newhit = oldhit.clone();
                    // loop all the contributions and add the time.
                    for (auto contrib: oldhit.getContributions()) {
                        auto newcontrib = contrib.clone();
                        newcontrib.setTime(contrib.getTime() + current_time_in_ns);
                        newhit.addToContributions(newcontrib);
                    }
                    calo_col.push_back(newhit);
                }
            } else {
                continue;
            }

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