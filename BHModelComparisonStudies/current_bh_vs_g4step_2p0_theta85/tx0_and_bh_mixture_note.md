# Personal Note: tX0 and the BH Mixture

`tX0` means material thickness measured in radiation lengths:

```text
tX0 = material thickness / radiation length X0
```

For electrons, radiation length is the natural material scale for bremsstrahlung. A larger `tX0` means the electron crossed more effective material, so the chance of radiating photons and losing momentum is larger.

The BH splitter predicts a distribution of retained momentum fraction:

```text
z = p_after / p_before
```

That distribution depends on `tX0`, so the model needs `tX0` as an input. In the current GSF code, each call to:

```cpp
BetheHeitlerSplitter::split(parent, tX0, bz)
```

recomputes the mixture for that specific `tX0`:

```cpp
auto mixture = bhMixture6(tX0);
```

So the BH mixture is not a fixed global distribution. It changes whenever the material thickness passed to the splitter changes.

## Current Code Behavior

The current `BetheHeitlerSplitter.cpp` has three regimes.

### 1. Negligible material: `tX0 < 1e-4`

One component:

```text
weight = 1.0
mean z = 1.0
variance = 0.0
```

This means no material effect and no momentum loss.

### 2. Thin CEPC material branch: `1e-4 <= tX0 < 0.1`

Two components:

```text
component 0: no-loss branch
component 1: bremsstrahlung tail branch
```

The code computes:

```text
expectedMean = exp(-tX0)
tailWeight   = clamp(10 * tX0, 0.02, 0.20)
tailMean     = clamp((expectedMean - (1 - tailWeight)) / tailWeight, 0.50, 0.999)
```

Then the components are:

```text
component 0:
  weight = 1 - tailWeight
  mean z = 1.0
  variance = 0.0

component 1:
  weight = tailWeight
  mean z = tailMean
  variance = tX0^2
```

As `tX0` increases, the tail branch becomes more important:

```text
tX0 = 0.001: tail weight 0.02, tail mean z about 0.950
tX0 = 0.010: tail weight 0.10, tail mean z about 0.900
tX0 = 0.030: tail weight 0.20, tail mean z about 0.852
```

The weighted mean is designed to stay close to:

```text
E[z] = exp(-tX0)
```

### 3. High-material branch: `tX0 >= 0.1`

The code switches to a six-component ACTS high-x polynomial table. It evaluates the table at:

```text
xx = min(tX0, 0.2)
```

For each component, the model computes:

```text
weight_i = polynomial_i(xx)
mean_i   = polynomial_i(xx)
var_i    = polynomial_i(xx)
```

Then all weights are normalized to sum to 1.

## Important Practical Point

In the GSF algorithm, the `tX0` passed to the splitter may be accumulated over multiple detector layers or hits before a split is triggered. Therefore the BH model is applied to the accumulated material since the previous split, not necessarily to every individual Geant4 material step.

For the material-step comparison study, we can still compare the model to G4 truth by evaluating the BH mixture at the G4 step-level `step_tX0`, then later decide whether we also need an accumulated-`tX0` comparison that better matches the GSF calling pattern.
