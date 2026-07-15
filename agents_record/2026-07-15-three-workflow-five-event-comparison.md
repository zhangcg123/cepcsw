# Three backward-workflow five-event comparison (2026-07-15)

The independent reverse filter, KL reduction-aware GSF smoother, and new
CMSSW-like GSF workflow were rerun on selected zero-based event indices
3, 5, 7, 9, and 11 from `/tmp/gsf-match-tracks.root`. All used the default
five-component CEPC BH model and `MaxComponents=24`. Reverse used BestBranch;
the KL smoother used WeightedMean; CMSSW-like output is its fixed innermost
mixture collapse. All five tracks completed in all workflows.

| index | truth | LCIO | reverse | KL smoother | CMSSW-like |
|---:|---:|---:|---:|---:|---:|
| 3 | 2.000359 | 1.798170 | 1.990557 | 1.797928 | 1.992258 |
| 5 | 2.000359 | 1.923763 | 2.010072 | 1.923712 | 2.017784 |
| 7 | 2.000359 | 2.000040 | 2.000306 | 1.999892 | 2.003260 |
| 9 | 2.000359 | 2.004091 | 2.004099 | 2.003979 | 2.007069 |
| 11 | 2.000359 | 1.793625 | 1.983018 | 1.793759 | 1.989450 |

All values are pT in GeV. Mean absolute fractional residuals on this deliberately
small focused sample are LCIO 4.8948%, reverse 0.4064%, KL smoother 4.8968%,
and CMSSW-like 0.4604%. RMS residuals are 6.6884%, 0.5025%, 6.6902%, and
0.5203%, respectively. This confirms the earlier observation: the KL smoother
is essentially LCIO-like, while both reverse refits recover the selected hard
losses. CMSSW-like is better than reverse on indices 3 and 11 but worse on 5,
7, and 9, so five events do not establish superiority.

Outputs are `/tmp/gsf-compare-{reverse,kl,cms}{,-flat}.root`. A tuple caveat
was observed: unselected entries after a selected event can repeat the previous
GSF values. Only the exact selected zero-based indices, corresponding to tuple
`iev` 4, 6, 8, 10, and 12, were used in the table.
