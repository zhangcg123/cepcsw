#ifndef DetSimSD_TpcPadRowTruth_h
#define DetSimSD_TpcPadRowTruth_h

struct TpcPadRowTruth {
  int trackID = -1;
  int firstStep = -1;
  int lastStep = -1;
  int hookStep = -1;

  void addStep(int owner, int step) {
    // Unfinished truth ranges must not survive a change of simulated particle.
    if (trackID != owner) {
      *this = {};
      trackID = owner;
    }
    if (firstStep < 0) firstStep = step;
    lastStep = step;
  }
};

#endif
