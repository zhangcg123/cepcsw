# Beam-guided free-loss objective: focused mechanical gate

The authorized experiment adds a second, default-on free-loss search. The
existing free-loss search minimizes the hit-only normalized likelihood. The
new search minimizes that same score plus one scalar transverse beam-origin
predictive likelihood from each trial's hit-only RTS IP state. No beam hit is
inserted into the KF, backward refilter, or RTS measurement sequence. The two
searches publish separate RTS/backward endpoint pairs and row-aligned flat
branches; when the new control is off, its endpoints copy base free-loss.

Steering: `FreeLossBeamSpotObjective=true` (also frozen by
`BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE=1` in the batch helper). Beam mean `(0,0)` mm;
horizontal/vertical widths `(0.0145,0.000036)` mm by default. The two searches
share `SigmaLogLoss`, selected interval, and all detector-hit fit settings.

Mechanics were tested with one 20-event tracker input from
`sim_large_barrel_20261001/sim-barrel-1.root`. The same installed code ran
entries 11, 16, 17 with the new switch on and off. Both jobs completed with
three ordinary successful rows. All 202 non-beam flat fields were identical
between the two runs. With the switch off, the beam-guided RTS/backward IP
parameters, covariances, local chi2 and score vectors copied base free-loss.
With the switch on, entries 16 and 17 produced distinct optimized pairs and
the saved `objective_nll2 = hit_nll2 + beam_nll2` closed. Entry 11 had no
selected breakpoint and copied ordinary results. Representative RTS pT (GeV):

| Entry | Truth | Base free-loss | Beam-guided free-loss |
|---:|---:|---:|---:|
| 11 | 26.792866 | 26.737669 | 26.737669 |
| 16 | 11.332882 | 11.308965 | 11.306710 |
| 17 | 35.528835 | 35.514193 | 35.513599 |

The 29 batch/card tests, RecBreakpointTransport and RecBreakpointLikelihood
tests, and expanded tuple score-closure check passed. The changed module and
generated configuration were installed package-locally. This is a mechanical
regression gate only. It neither shows a physics improvement nor establishes
clean-track or tail safety. A categorized same-code population comparison is
still required before any performance claim.
