# Stable names for CEPCSW tracking

These names separate concepts; they do not establish equality between scores.
Current source determines what a field contains. Preserve agreed symbols;
do not introduce symbols merely to restate these definitions.

| Canonical name | Meaning |
|---|---|
| Forward-predicted state | State at a hit before using that hit, obtained by propagation from the preceding forward-updated state. |
| Forward-updated state | State after using that hit. Forward-filtered state is an alias; map once, then use forward-updated state. |
| RTS-smoothed state | State at a hit after RTS incorporates later-hit information through the buffered forward model; not an independent backward-filter state. |
| Residual | Difference between two explicitly identified quantities at specified locations. A residual is not a chi2 value. |
| Forward-predicted measurement residual | Measured coordinates minus those predicted from the forward-predicted state. Innovation is an alias; do not alternate names. |
| Smoothed measurement residual | Measured coordinates minus those predicted from the RTS-smoothed state. Specify captured linearized versus native measurement evaluation. |
| Smoothed propagation residual | Destination RTS-smoothed state minus the preceding RTS-smoothed state transported with the specified model and fixed trial loss. |
| Seed residual | First RTS-smoothed state minus the initial seed mean, with matching parameterization and pivot. |
| Chi2 contribution | Scalar residual^T * inverse covariance * residual. Identify that covariance. Penalty and quadratic contribution are aliases, not additional quantities. |
| Forward innovation chi2 sum | Sum of chi2 contributions from forward-predicted measurement residuals. Prediction covariance contains propagated seed/state uncertainty and process noise. Not a direct complete residual score of the stored forward-updated trajectory. |
| Complete smoothed chi2 | Sum of smoothed measurement, smoothed propagation and seed chi2 contributions. Measurement-only smoothed chi2 is different. |
| Global measurement chi2 | Measurement residuals relative to the joint Gaussian model mean, weighted with their full joint covariance. Specify the model; do not silently identify it with a native forward field. |
| Minuit objective | Complete scalar actually returned to Minuit at one trial loss. List its terms; do not abbreviate it to chi2 if other terms are included. |
| Trial loss | Loss parameter currently evaluated. In maintained free fitting, b is a log momentum-loss parameter, not a percentage; verify its mapping when needed. |

## Guardrails for this discussion

- Use chi2 consistently for covariance-weighted squared-residual quantities.
  If code calls a field quadratic, map it once to the relevant chi2 name.
  Do not call a residual vector and its chi2 contribution the same thing.
- Never say "forward and smoothed are the same". Identify whether comparing
  states, residuals, per-hit chi2, complete chi2, Minuit objectives, fitted
  losses or final IP momenta.
- Distinguish measurement/process noise, predicted/updated/smoothed state
  covariances, and residual covariances. Do not substitute them by analogy.
  Singular noise requires constraints on unsupported directions.
- Score comparisons retain the same loss, hits, seed treatment, noise and
  model unless a difference is the explicit subject. Comparing at fixed loss
  does not establish equal minimizing losses for different objectives.
- This glossary prescribes no answer to the disputed equality. Inspect code
  and establish assumptions before claiming a mathematical or numerical match.
