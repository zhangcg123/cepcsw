#ifndef RecGsfTracking_BetheHeitlerSplitter_h
#define RecGsfTracking_BetheHeitlerSplitter_h

#include <vector>

struct GsfComponent;

/// Bethe-Heitler bremsstrahlung splitter using the ACTS
/// AtlasBetheHeitlerApprox<6,5> parameterization data (embedded).
///
/// Three regimes:
///   tX0 < 0.0001  →  no splitting (1 component, no energy loss)
///   0.0001 ≤ tX0 < 0.002  →  1 component (single Gaussian approx)
///   0.002 ≤ tX0 < 0.1    →  6 components (low-x parameterization)
///   0.1 ≤ tX0             →  6 components (high-x, capped at 0.2)
///
struct BetheHeitlerSplitter {
  BetheHeitlerSplitter();

  /// Split a component at a surface with radiation thickness tX0.
  /// @param parent  The component to split (modified in-place for i=0)
  /// @param tX0     Path length in radiation lengths
  /// @param bz      B-field strength [T] for helix-to-kappa conversion
  /// @return Vector of child components (parent is first element)
  std::vector<GsfComponent*> split(GsfComponent* parent, double tX0, double bz) const;
};

#endif
