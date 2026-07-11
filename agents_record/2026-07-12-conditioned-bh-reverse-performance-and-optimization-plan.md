# Conditioned-BH reverse performance and optimization plan

Date: 2026-07-12

## Outgoing execution focus and evidence

The completed focus was execution proof of the 2 GeV pT, 85-degree,
step-`t/X0`-conditioned electron BH pipeline. Ten 1000-event Geant4 jobs
produced 2,574,697 owned transitions (257.47 surface-to-surface intervals per
event), including 9,985 eBrem-containing intervals and 0.131152 GeV mean
eBrem-attributed event loss. Of these, 9,528 eBrem rows lie inside the first
reconstruction range `t/X0 < 0.03`; 783 thicker rows were retained in the
source but excluded from that table. The exact source ROOT files, transition
CSVs, and audit JSON files are under
`BHModelComparisonStudies/CEPC2GeV85StepConditioned/production/` and must not
be paired with older nominally matching tracking files.

The five-stratum extraction and constrained eight-knot, five-component fit
produced finite normalized artifacts with an explicit zero-material,
vanishing-tail limit under
`Reconstruction/RecGsfTracking/data/CEPC2GeV85StepConditioned/`. The model is
integrated as `BHModel="CEPC2GeV85StepConditioned"`. Forward event 11 retained
234/234 hits with 1047/0/0 accepted/recovered/rejected updates but remained at
1.7934 GeV pT, essentially LCIO instead of 2.0004 GeV truth. Detailed
production and spectrum provenance is in
`2026-07-12-2gev85-transition-pipeline-and-state-diagnosis.md`.

`MaterialPathMode="DD4hepBetweenSurfaces"` resolved material ownership. In
matched event 11, 232 transitions total 0.0737544 X0 in Geant4 and 0.0739544
X0 in DD4hep (ratio 0.99730). The hard ITK transition is 0.00719995 versus
0.00720455 X0 (ratio 0.99936). The legacy current-surface sum is only
0.0191777 X0; crossed-cradle sums remain diagnostic, not authoritative.

The first truth-to-branch diagnosis was invalid because Geant4 truth and the
refit came from different detector realizations of the same generated event.
The corrected `/tmp/gsf-match-tracks.root` run showed that the hard hit-6
interval did not split at `MaxComponents`; after cutoff it split one surface
late. Transition 150 remained below the fixed threshold. It retained 234/234
hits with 929/0/0 updates but stayed at LCIO pT.

An independent exact-pair seed 1 event 3 has true `z=0.910276` on hit 6->7,
233 hits, and matching MC and detector-hit signatures. Before the ordering
fix, 12 existing components prevented children there. Split-before-budget now
expands 12 parents to 60 children at hit 6, then cutoff and same-surface KL
reduction return to 12. The truth-covering `z~=0.899` child survives at weight
0.0165 and reaches 0.917 after hit 7. Forward publication nevertheless stays
at LCIO because its inner filtered history does not carry that downstream loss
choice back to the IP.

Retained-lineage RTS smoothing with TopN completed all 12 lineages, including
the correctly timed loss lineage, but changed inner pT only from 1.7980208 to
about 1.79809 GeV; the final best TopN lineage used a low-loss child. ACTS
inspection then established that its GSF performs a second backward
multi-component filter rather than component RTS smoothing.

The local reverse pass was aligned in ordering, direction-aware DD4hep
intervals, split-before-budget, KL reduction, and IP output. Its first run
terminated after hit 6 because recursive KL ancestry strings exhausted memory;
bounded diagnostic histories repaired this without changing filter
mathematics. The completed paired run retained 233/233 hits, accepted/rejected
2457/0 reverse updates, made seven reverse splits/reductions, and gave a
moment-matched 11-component IP pT of 1.99286 GeV versus 2.0004 truth and 1.7980
LCIO. This was paired execution evidence, not statistical validation.

Three further paired results were mixed: seed/event 1/5 gave 2.0302 GeV,
2/7 stayed at 1.8538 versus 1.8521 LCIO, and 7/9 gave 2.0047 GeV. In the
early-loss failure, correct approximately 2.06 GeV children existed, but
adjacent-hit likelihoods did not separate them from the no-loss state and the
BH prior left the no-loss endpoint at weight 0.977. A deliberately selected
20-event production sample spanning principal `z=0.555-0.991` improved
absolute pT error in 19/20: mean 77.3->32.8 MeV, median 36.7->15.3 MeV, with
13/20 inside 20 MeV and 17/20 inside 50 MeV. All 16 inner-transition cases
improved; the dominant failure was a penultimate-transition loss with too
little downstream lever arm. This was neither unbiased nor held out.

The outgoing ordered work was to repeat the paired hard-event split audit,
audit the completed reverse mixture across hit 6, audit the fixed split
threshold separately, and run verbose events 11/16/17 with 4-5 hypotheses and
finite 234-hit output. Its execution success criterion was a finite normalized
producer-to-fit-to-GSF pipeline and finite 234-hit events without covariance
failure or update rejection, explicitly not predictive validation. Non-goals
were global tuning, premature runtime pruning, treating delayed TopN as final,
SimHit momentum fitting, ACTS coefficients as CEPC validation, broad claims,
and additional shared-package changes.

## New categorized performance evidence

A new exactly matched sample consists of 100 files with 10 events each. Using
primary-electron Geant4 eBrem steps in tracker volumes and a 10% single or
cumulative hard-loss boundary gives 381 no-eBrem, 457 light-eBrem, and 162
hard-eBrem events.

For no-eBrem events, LCIO has median residual -0.0190% and central-68% width
0.2929%. The reverse weighted mixture shifts to +0.2307% and width 0.5786%; the
reverse best branch improves this to +0.1047% and width 0.3446%, but remains
worse than LCIO. All three have 370/381 inside 5%; inside 1% the counts are
364 LCIO, 338 weighted, and 356 best branch.

For hard-eBrem events, reverse best-branch execution succeeds for 161/162; seed
74 entry 4 loses all forward components at hit 4. On the common 161, LCIO,
weighted reverse, and best reverse have median residuals -10.459%, -0.0310%,
and -0.1678%; RMS values 32.315%, 29.726%, and 31.005%; and counts inside 1%
of 59, 84, and 88. Both reverse outputs recover many hard losses. Best branch
better protects the no-eBrem core and is therefore now the default, while
`ReverseOutputMode="WeightedMean"` remains selectable.

## Recorded optimization plan and rationale

The current problem is performance optimization, not basic execution: the
algorithm often selects useful hard-loss corrections, but applying the same
probabilistic inverse-loss process also biases and broadens tracks with no true
tracker eBrem. Changing only the final mixture summary does not remove this.

Proceed in this order:

1. Verify and, if necessary, add an exact identity/no-eBrem component with
   retained fraction 1 and near-zero conditional variance. Its probability
   must be derived from all Geant4 transitions, including transitions without
   eBrem, and approach one as `t/X0 -> 0`. Repeated components whose means are
   merely close to one can accumulate a positive reverse-pT bias.
2. Preserve the identity lineage through cutoff and KL reduction rather than
   merging it with nearby loss components. Otherwise the nominal no-loss
   hypothesis itself moves below retained fraction 1 and clean tracks cannot
   remain unchanged.
3. Run an otherwise identical reverse fit with radiative BH convolution
   disabled on the categorized no-eBrem sample. This separates degradation
   intrinsic to the second reverse refit from degradation caused by the BH
   mixture and reduction.
4. If the no-BH reverse control still degrades the core, replace the current
   second-refit reuse of forward-filtered measurement information with a
   statistically consistent forward/backward message or smoother formulation.

The previously suggested additional measurement-evidence selection threshold
is intentionally not part of this plan, per user direction. The plan first
tests and repairs the physical identity hypothesis and then isolates whether
the remaining bias comes from BH processing or the reverse formulation.

Success means the best-branch reverse output retains the demonstrated hard-loss
recovery while matching, rather than broadening or biasing, the LCIO no-eBrem
core. No broad validation claim follows until this is reproduced on independent
samples and broader energy/angle coverage.
