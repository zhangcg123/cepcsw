#ifndef RecGsfTracking_BetheHeitlerSplitter_h
#define RecGsfTracking_BetheHeitlerSplitter_h

#include <string>
#include <vector>

struct GsfComponent;

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
    CEPC2GeV85StepConditioned6
  };

  BetheHeitlerSplitter();
  explicit BetheHeitlerSplitter(Model model);
  explicit BetheHeitlerSplitter(const std::string& modelName);

  static Model modelFromName(const std::string& modelName);
  static const char* modelName(Model model);

  /// Split a component at a surface with radiation thickness tX0.
  /// @param parent  The component to split (modified in-place for i=0)
  /// @param tX0     Path length in radiation lengths
  /// @param bz      B-field strength [T] for helix-to-kappa conversion
  /// @return Vector of child components (parent is first element)
  std::vector<GsfComponent*> split(GsfComponent* parent, double tX0, double bz,
                                   bool reverse = false) const;

private:
  Model m_model = Model::CEPC2GeV85StepConditioned;
};

#endif
