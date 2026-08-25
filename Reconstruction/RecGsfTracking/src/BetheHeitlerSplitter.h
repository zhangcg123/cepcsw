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
/// The default model preserves the current CEPC thin-material test behavior.
/// A simulation-derived global model can be selected in parallel.
/// A faithful ACTS default AtlasBetheHeitlerApprox regime selection is also
/// available as ActsAtlas; its ATLAS-derived coefficients are not CEPC
/// validation.
/// CEPC2GeV85StepConditioned is the explicitly scoped five-component,
/// transition-t/X0-conditioned execution model fitted to the 2 GeV, 85-degree
/// primary-electron sample; it is not a general or validated CEPC model.
/// CEPC2GeV85StepConditioned6 is a parallel six-component extraction from the
/// same sample. It replaces the original 1--5% and 5--20% components with
/// separate 1--5%, 5--10%, and 10--20% components.
/// CEPCRuntimeGenericGrid5Clear and CEPCRuntimeCategoryAligned5Clear are
/// default-off five-component interval-level candidates fitted to exact
/// DD4hepBetweenSurfaces runtime paths and matched aggregate Geant4 eBrem
/// losses in the topology-clear control population. They differ only in their
/// generic-logarithmic versus detector-interval-aligned t/X0 knot grids.
/// CEPCRuntimeCategoryAligned9Clear keeps the latter knot grid and fitted
/// per-cell loss probabilities, but replaces the four broad radiative modes
/// with fixed proposals centered at 1, 3, 5, 7, 9, 15, 30, and 70 percent
/// aggregate loss. It is a default-off experimental control.
/// CEPCRuntimeCategoryAligned15Clear keeps the same t/X0 knot grid and total
/// radiative probability, but uses one exact identity atom plus fourteen
/// nonuniform radiative proposals: five in 0--1%, six in 1--6%, one in
/// 6--10%, and two in 10--100%. Adjacent two-sigma proposal bounds meet. It is
/// also a default-off experimental control.
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
    CEPC2GeV85StepConditioned,
    CEPC2GeV85StepConditioned6,
    CEPCRuntimeGenericGrid5Clear,
    CEPCRuntimeCategoryAligned5Clear,
    CEPCRuntimeCategoryAligned9Clear,
    CEPCRuntimeCategoryAligned15Clear
  };

  BetheHeitlerSplitter();
  explicit BetheHeitlerSplitter(Model model);
  explicit BetheHeitlerSplitter(const std::string& modelName);

  static Model modelFromName(const std::string& modelName);
  static const char* modelName(Model model);

  /// Return the configured mixture without applying it to a track state.
  /// This is used by hypothesis-level diagnostics/refitters that must score
  /// the BH prior before choosing which deterministic process to propagate.
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
  Model m_model = Model::CEPC2GeV85StepConditioned;
};

#endif
