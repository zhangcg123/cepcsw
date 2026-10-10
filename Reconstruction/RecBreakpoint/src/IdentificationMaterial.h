#pragma once

#include "DD4hep/DD4hepUnits.h"
#include "DDRec/MaterialManager.h"
#include "TVector3.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace breakpoint {
// A reconstructed geometry feature, not the curved KF propagation path.
// The coverage repair follows GsfAlgorithm::geometryTransitionMaterialPath:
// TGeo can omit the starting volume when the endpoint is on a boundary.
struct IdentificationMaterial {
  double tx0 = std::numeric_limits<double>::quiet_NaN();
  double coveredMM = 0.;
  int segments = 0;
  int status = 0; // 1 complete, 2 complete after boundary repair, -1 invalid
};

inline IdentificationMaterial identificationMaterial(
    dd4hep::rec::MaterialManager &manager, const TVector3 &from,
    const TVector3 &to) {
  IdentificationMaterial invalid;
  invalid.status = -1;
  const double length = (to - from).Mag();
  if (!std::isfinite(length) || length <= 0.) return invalid;
  auto point = [](const TVector3 &p) {
    return dd4hep::rec::Vector3D(p.X() * dd4hep::mm,
                               p.Y() * dd4hep::mm, p.Z() * dd4hep::mm);
  };
  auto scan = [&](const TVector3 &begin) {
    IdentificationMaterial result;
    result.tx0 = 0.;
    result.status = 1;
    for (const auto &segment : manager.materialsBetween(point(begin), point(to))) {
      if (!(segment.second > 0.)) continue;
      result.coveredMM += segment.second / dd4hep::mm;
      ++result.segments;
      const double x0 = segment.first.radLength();
      if (!(x0 > 0.) || !std::isfinite(segment.second / x0))
        result.status = -1;
      else result.tx0 += segment.second / x0;
    }
    return result;
  };
  try {
    auto result = scan(from);
    const double tolerance = std::max(1.e-3, 1.e-6 * length);
    if (std::abs(result.coveredMM - length) > tolerance) {
      const double nudge = std::min(1.e-3, .01 * length);
      const auto direction = (to - from).Unit();
      result = scan(from + nudge * direction);
      const double x0 = manager.materialAt(point(from + .5 * nudge * direction)).radLength();
      if (result.status == 1 && x0 > 0. &&
          std::abs(result.coveredMM - (length - nudge)) <= tolerance) {
        result.tx0 += nudge * dd4hep::mm / x0;
        result.coveredMM += nudge;
        ++result.segments;
        result.status = 2;
      } else result.status = -1;
    }
    if (result.status < 0 || !std::isfinite(result.tx0)) {
      result.status = -1;
      result.tx0 = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
  } catch (...) { return invalid; }
}
} // namespace breakpoint
