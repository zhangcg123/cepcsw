# Common inward filter and unconditional side products

Status: the common inward filter and unconditional passive records remain
active. The former CMS-like hit-1 publication described below is superseded by
`2026-08-29-smoothed-diagnostic-only-publication.md`; the original evidence is
preserved here for provenance.

## Decision

Reverse and CMS-like now use one `runGsfInwardFilter` implementation for the
complete inward pass. The method-specific publication code no longer owns a
second backward loop. At every successfully processed inward surface the
function saves the accepted backward-predicted Gaussian snapshots needed for
the same-surface product with the stored forward-updated mixture.

The product is not optional and has no configurable switch. After the live
inward recursion is complete, the common function materializes, normalizes,
cuts, and reduces every

```text
S[i] = F_updated[i] x B_predicted[i]
```

side mixture. Delaying this matrix algebra until the live recursion has
finished makes the non-steering boundary explicit: no side product can affect
a later measured-hit update, posterior cutoff, KL reduction, or propagation.

The live and passive flows are therefore:

```text
B_updated[i+1]
  -> material/BH propagation
  -> B_predicted[i]
  -> real hit i
  -> B_updated[i]
  -> posterior cutoff/KL
  -> next inward surface

after the live pass:
  F_updated[i] x saved B_predicted[i]
  -> passive side-mixture cutoff/KL
  -> lineage/flat-tuple record
```

Reverse still publishes the terminal backward mixture. CMS-like still
publishes `S[1]` and falls back to the terminal backward mixture only when no
valid hit-1 product can be formed. Thus the side information is shared while
the established method endpoints remain distinct.

## Persistence contract

No property, handle, branch, or numeric schema code was added. The existing
default-on lineage EDM and flat-tuple vectors carry the side record for both
reverse and CMS-like:

- node source `3` means a common inward product/reduction node;
- node operation `5` means an evaluated product candidate;
- edge operation `5` connects its source-1 forward node and source-2 backward
  measurement node;
- the source-2 parent contributes its exact persisted predicted state, not its
  filtered state;
- fate `7` means a retained inward side-mixture message that is not in the
  published endpoint.

Every accepted product candidate, including candidates later cut or merged,
remains in the DAG. The reduced side survivors are also retained. Product
components are transient C++ objects and are deleted after EDM publication;
the tuple stores immutable values rather than live pointers.

## Mechanical validation

`RecGsfTracking` and `RecGsfFlatTuple` built and installed in the focused
EL9/LCG-105 tree. The build briefly produced a zero-byte generated
`GsfAlgorithm.cpp.o` because two rebuilds overlapped; deleting that one build
artifact and rebuilding serially restored the 2.2 MB object, the 1.7 MB shared
library, and both generated Gaudi configurables. This was a build-tree event,
not a source or data failure.

A comprehensive focused gate used job 98 entry 15 with
`CEPCRuntimeCategoryAligned15Clear`, `MaxComponents=10`, cutoff `1e-4`, the
standard initializer, and inward covariance scale 100. It recorded 18,375
operation-5 candidates over all 232 processed inward surfaces. Moving side
materialization after the live pass left the reverse endpoint and all 8,496
source-1/source-2 statistical nodes exactly unchanged:

| endpoint | reverse pT (GeV) |
|---|---:|
| BestBranch | 25.4029474938053 |
| WeightedMean | 25.6714106103170 |
| FullMixtureMode | 25.3928210510374 |

At equal covariance scale the CMS-like run had the same source-1/source-2
statistical sequence and published source `3` from hit 1:

| endpoint | CMS-like pT (GeV) |
|---|---:|
| BestBranch | 25.4025089750094 |
| WeightedMean | 25.6524658634538 |
| FullMixtureMode | 25.3926802092713 |

The maintained five-component baseline was then rerun from the same installed
binary on canonical hard-loss events 11, 16, and 17 with
`MaxComponents=12`, cutoff `5e-3`, the standard initializer, and equal inward
covariance scales of 100. Both workflows completed, had the same ordered
forward/backward node structure and local innovation/kappa values, and
recorded side products. Terminal source-2 `weight` annotations differ where
reverse marks its final renormalized survivors while CMS-like marks those same
nodes as internal messages; this is endpoint annotation, not a second inward
calculation.

| event | truth pT | LCIO pT | reverse FullMixtureMode | CMS FullMixtureMode | hit-1 product candidates |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.731567 | 40.895454 | 40.909320 | 40.909266 | 60 |
| 16 | 37.894016 | 18.292832 | 18.318878 | 18.318878 | 61 |
| 17 | 18.796978 | 14.806669 | 18.620237 | 18.620109 | 55 |

These are mechanical gates. They do not establish a CMS-like physics gain or
replace the required topology-clear no/light/hard-loss population study.

## Follow-up

Use the now-common per-surface side mixtures to compare backward-only and
forward-by-backward posterior rank at the first truth-compatible lineage
crossover. Decompose changes into BH prior weight, exact local `dchi2`, and
`logDetInnovation`; keep the product passive until a separately reviewed
algorithmic use is justified. The material/BH consistency and BH-variance
questions remain active and unchanged.

The earlier forward-sharing checkpoint remains in
`agents_record/2026-08-28-shared-forward-reverse-cms-framework.md`. Historical
material, BH, truth-oracle, ECAL, global-loss, and population evidence formerly
listed verbosely in `AGENTS.md` remains in its dated `agents_record/` entries;
this record changes no conclusion or production default from those studies.
