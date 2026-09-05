#ifndef RecGsfTracking_BetheHeitlerSplitter_h
#define RecGsfTracking_BetheHeitlerSplitter_h

#include <string>
#include <vector>

struct GsfComponent;

/// Exact Gaussian-mixture parameters returned by the selected BH model.
/// `mean` and `variance` describe the retained-momentum fraction
/// z=p_after/p_before before it is applied to a track component.
struct BetheHeitlerMixtureComponent {
  double weight = 0.0;
  double mean = 1.0;
  double variance = 0.0;
};

/// Bethe-Heitler bremsstrahlung splitter.
///
/// CEPCRuntimeCategoryAligned9Clear is the compiled default. Despite its
/// retained historical selector name, it contains one effective identity
/// component plus nine globally optimized radiative Gaussians. Their means
/// and widths are shared across eight interval-aligned t/X0 knots; their
/// priors are knot-local. The likelihood fit used aggregate Geant4 eBrem
/// losses from 0.2% through 100%, while the Gaussian PDFs remain untruncated.
/// This all-simulation candidate is not a validated production BH model.
///
/// ActsAtlas is the only alternative. It reproduces the ACTS default
/// AtlasBetheHeitlerApprox regime selection; its ATLAS-derived coefficients
/// are a control rather than CEPC validation.
///
/// ActsAtlas regimes:
///   tX0 < 0.0001  →  no splitting (1 component, no energy loss)
///   0.0001 ≤ tX0 < 0.002  →  1 component (single Gaussian approx)
///   0.002 ≤ tX0           →  6 transformed polynomial components,
///                            evaluated at min(tX0, 0.2)
///
struct BetheHeitlerSplitter {
  enum class Model {
    ActsAtlas,
    CEPCRuntimeCategoryAligned9Clear
  };

  BetheHeitlerSplitter();
  explicit BetheHeitlerSplitter(Model model);
  explicit BetheHeitlerSplitter(const std::string& modelName);

  static Model modelFromName(const std::string& modelName);
  static const char* modelName(Model model);

  /// Return the configured mixture for a material interval.
  std::vector<BetheHeitlerMixtureComponent> mixture(double tX0) const;

  /// Split a component at a surface with radiation thickness tX0.
  /// @param parent  The component to split (modified in-place for i=0)
  /// @param tX0     Path length in radiation lengths
  /// @param bz      B-field strength [T] for helix-to-kappa conversion
  /// @return Vector of child components (parent is first element)
  std::vector<GsfComponent*> split(
      GsfComponent* parent, double tX0, double bz, bool reverse = false,
      std::vector<BetheHeitlerMixtureComponent>* returnedMixture = nullptr) const;

  /// Apply one deterministic retained-momentum fraction instead of querying
  /// the configured BH parameterization.  This is the process-level primitive
  /// used by the default-off truth-oracle diagnostic; the normal continuation
  /// propagation and downstream GSF workflow are unchanged.
  std::vector<GsfComponent*> splitWithRetainedFraction(
      GsfComponent* parent, double retainedFraction, double bz,
      bool reverse = false,
      std::vector<BetheHeitlerMixtureComponent>* returnedMixture = nullptr) const;

private:
  Model m_model = Model::CEPCRuntimeCategoryAligned9Clear;
};

#endif
