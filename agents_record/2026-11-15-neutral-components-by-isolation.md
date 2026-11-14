# Neutral-cluster energy components versus isolation — 2026-11-15

Two stacked-bar plots use the existing 55,391 unique neutral candidates from
500 current REC_ECALTRUTH files / 100,000 input events. Reuses verified native
truth and R1000 caches; no simulation, reconstruction or fit is rerun. All 500
REC file sizes/mtimes match the source identities in the native-read report.
No deleted BP tuple is used. Candidate selection stays fixed: selected Z->ee
tracks, 25/200 mrad collection half-windows and the existing upper-2% energy
cap of 55.75722694396976 GeV. No additional truth-origin filter is applied.

Isolation: I1000 = E_C/(E_C + sum_other E_j), over all valid other ECAL clusters
with opening angle <1 rad between nominal-IP-to-cluster-centre rays. The
candidate itself is omitted; charged electron clusters are always included
when inside the cone. No electron-excluded metric or curve is produced.

## Bars and denominators

Eight disjoint isolation intervals: [0,.1), [.1,.25), [.25,.5), [.5,.75),
[.75,.9), [.9,.95), [.95,.99), [.99,1]. Counts are respectively
24523/13436/11674/4357/868/142/193/198, summing to 55,391.

For EACH candidate, the native linked MC weights are normalized by that
candidate's total linked weight. The attributed measured energy is
E_C*w_component/sum(w); dividing by that candidate's measured E_C gives its
component fraction. Each bar is the arithmetic MEAN of those per-cluster
fractions. No summed-energy numerator/denominator or energy weighting is used.
Every bar sums to 100%. Values above bars are the mean largest INDIVIDUAL
identified tagged-photon fraction, not the sum FSR+eBrem. Sample counts appear
below every bar. Black residuals have no percentage label.

Plot neutral_components_vs_iso_R1000 separates FSR, eBrem, selected electrons,
and Others. FSR/eBrem here are explicitly origin components, not individual
photon purity. Plot neutral_individual_photons_vs_iso_R1000 separates photon
#1, photon #2, remaining tagged photons, selected electrons and Others. Photon
ranks are within each candidate by attributed fraction; they are not the same
identity across clusters. Each uniquely identified original photon includes its
uniquely assigned descendants. Different roots remain separate even when both
are FSR or both eBrem. Only radiation associated with the two selected Z->ee
electrons is tagged; untagged photons can remain in Others. Selected electrons
means direct native MC contribution from either selected production electron,
not all shower electrons. Others is the residual response, including unrelated
origins and unresolved ancestry; it is black.

## Findings from the native map

Mean largest-photon fractions by increasing isolation bin:
38.4/53.4/60.2/68.2/60.3/15.5/6.7/9.3 percent.
The native selected-electron fractions are
41.7/38.8/33.7/26.5/35.9/83.7/92.2/89.3 percent.
For I>=.99 (198 candidates), mean FSR=.8749%, eBrem=9.8381%, selected
electron=89.2511%, Others=.03584%; mean largest individual photon=9.2983%.
Only 3/198 exceed 95% for one tagged photon. High isolation therefore does
not identify predominantly tagged-photon response in this selected pool.
The saved labels point mainly to electron-attributed neutral candidates in
this extreme bin; this does not establish a physical misclustering rate or
prove an MC-link error. In particular the source audit did not demonstrate
that inherited projection fractions are wrong: simultaneous shared responses
can genuinely be inseparable. Physical spatial purity remains unvalidated.

## Verification and scope

analyze.py asserts exact candidate-key joins, source identities, scalar
isolation closure from candidate/neighbor energies, individual-photon table
max/second/sum closure and per-origin bounds, nonnegative per-cluster and
bar components, per-cluster/bar sums, count partition, and count-weighted
recovery of the inclusive arithmetic means. Per-bin numbers are in profiles.csv;
source/cache/code and plot hashes are in summary.json and plot_files.json.
The independent scalar recomputation is in verification.json. Both PNGs are
visually reviewed. PNG/PDF pairs use AGORA simulation and include the isolation
definition, cone radius, component legend and sample counts.

Gallery addition is a new two-plot section, using the plot-gallery skill and
one dedicated gallery agent. Preserve all 102 previous PNG/PDF hashes and all
17 previous section objects; expected final 77 PNGs/18 sections/29 PDFs. No
old removed section or electron-excluded variant is restored. The existing
localhost8000 server is reused if healthy; HTTP/hash and exact-once checks are
in gallery_verification.json. Analysis scripts/data/plots/manifests remain
uncommitted. No fitting/PFA source, card, build, install, default or production
change. The earlier truth-map ambiguity has not been resolved by this plot.

## Project-memory retention

Outgoing AGENTS.md preserved byte-for-byte in
2026-11-15-agents-before-neutral-components-by-isolation.md. Current focus
replaced; active laws/build instructions unchanged. Global ECAL status clarified
that shared-response inheritance is not by itself proof of erroneous weights.
