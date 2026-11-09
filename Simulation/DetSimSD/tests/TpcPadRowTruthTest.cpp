#include "TpcPadRowTruth.h"

#include <cassert>
#include <iostream>

int main() {
  TpcPadRowTruth truth;
  truth.addStep(4, 488);
  truth.hookStep = 488;
  truth.addStep(4, 489);
  assert(truth.trackID == 4 && truth.firstStep == 488 &&
         truth.lastStep == 489 && truth.hookStep == 488);

  // An ordinary row exit/event boundary starts an independent range.
  truth = {};
  truth.addStep(4, 490);
  truth.addStep(3, 130);
  assert(truth.trackID == 3 && truth.firstStep == 130 &&
         truth.lastStep == 130 && truth.hookStep == -1);
  truth.addStep(3, 131);
  truth.hookStep = 131;
  truth.addStep(3, 132);
  assert(truth.firstStep == 130 && truth.lastStep == 132 && truth.hookStep == 131);

  // The other observed reversed range, 459 -> 170.
  truth = {};
  truth.addStep(4, 459);
  truth.addStep(5, 168);
  truth.addStep(5, 169);
  truth.hookStep = 169;
  truth.addStep(5, 170);
  assert(truth.firstStep == 168 && truth.lastStep == 170 && truth.hookStep == 169);

  // A new particle also invalidates an old hook; it must cross the centre itself.
  truth.addStep(6, 200);
  assert(truth.firstStep == 200 && truth.lastStep == 200 && truth.hookStep == -1);
  truth.hookStep = 200;
  truth.addStep(7, 200);
  assert(truth.firstStep == 200 && truth.lastStep == 200 && truth.hookStep == -1);

  // Resuming a previously visited particle cannot reuse the intervening range.
  truth.addStep(5, 171);
  assert(truth.trackID == 5 && truth.firstStep == 171 && truth.lastStep == 171 &&
         truth.hookStep == -1);
  truth = {};
  truth.addStep(5, 1);
  assert(truth.firstStep == 1 && truth.lastStep == 1 && truth.hookStep == -1);
  std::cout << "TPC truth ownership, range, hook and reset checks passed\n";
}
