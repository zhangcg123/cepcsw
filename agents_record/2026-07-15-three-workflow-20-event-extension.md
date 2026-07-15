# Three-workflow 20-event extension (2026-07-15)

All ten events in preserved files `gsf-31.root` and `gsf-52.root` were rerun
with MaxComponents=24 and the default CEPC five-component BH model. The three
configurations match the prior five-event comparison: reverse BestBranch, KL
smoother WeightedMean, and CMSSW-like innermost mixture. All workflows produced
outputs for every event.

On all 20 tuple entries, mean absolute pT residuals are 9.3110% LCIO, 8.4094%
reverse, 9.3082% KL, and 8.3671% CMSSW-like. RMS values are 25.7836%, 25.3335%,
25.7888%, and 25.3040%. These inclusive numbers are dominated by three short
or mismatched tracks with only 36, 67, and 115 hits; two have reconstructed pT
near 0.27 and 0.55 GeV against 2 GeV truth in every method.

For the 17 entries with more than 200 hits:

| method | median residual | mean absolute | RMS | central-68 half-width | inside 1% | inside 2% |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | -0.0388% | 1.5022% | 4.4080% | 0.5131% | 14/17 | 15/17 |
| reverse | -0.0289% | 0.5045% | 1.3265% | 0.2353% | 15/17 | 16/17 |
| KL smoother | -0.0448% | 1.4986% | 4.4058% | 0.5081% | 14/17 | 15/17 |
| CMSSW-like | +0.0461% | 0.4690% | 1.0942% | 0.2560% | 15/17 | 16/17 |

CMSSW-like beats ordinary reverse by absolute truth error on 7/17 long-track
entries and reverse beats CMSSW-like on 10/17. On all 20 it is exactly 10/10.
Thus CMSSW-like has slightly better aggregate tail metrics here, while ordinary
reverse wins more of the normal long-track cases. The KL smoother again tracks
LCIO closely.

Outputs are `/tmp/gsf-compare20-s{31,52}-{reverse,kl,cms}{,-flat}.root`.
This is a small convenience sample, not categorized or held-out validation.
