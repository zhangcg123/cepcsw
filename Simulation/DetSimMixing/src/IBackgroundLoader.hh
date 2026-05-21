#ifndef IBackgroundLoader_hh
#define IBackgroundLoader_hh

struct BackgroundEvent;

class IBackgroundLoader {
public:
    virtual ~IBackgroundLoader() = default;

    // the loader is responsible to unpack the hits into background event.
    // if failed, then return false.
    virtual bool fill(BackgroundEvent& bkg_event, const double current_time_in_ns) = 0;
};

#endif