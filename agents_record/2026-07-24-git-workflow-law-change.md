# Git workflow law change

Date: 2026-07-24

The user explicitly replaced the previous active law:

> Do not perform Git operations during optimization unless the user explicitly
> requests a specific Git action.

The new active law requires frequent Git use during development: inspect
status and diffs, and make focused checkpoint commits after coherent source or
documentation changes and proportionate verification. Generated ROOT files,
logs, plots, tables, notebooks, and batch cards remain separate from
source/documentation commits. Pushing and branch changes still require an
explicit user request.

Later on 2026-07-24, the user clarified that `optimizing` has not been the
working branch for a long time. Read-only inspection confirmed that no local
`optimizing` branch exists; the current local branch is `dev`. A stale-looking
`origin/optimizing` remote-tracking reference still exists, but it does not
override the user's branch correction.

The obsolete `optimizing` law was therefore replaced with: use `dev` as the
active development branch, and do not switch, create, rename, delete, merge,
or rebase branches without an explicit request for that operation.

Later on 2026-07-24, the user narrowed the routine commit scope to the core
project implementation. The active law now permits routine checkpoint commits
only for C++ source and header files under
`Reconstruction/RecGsfTracking/src/`. Documentation, status records, run
cards/options, scripts, build files, generated ROOT files, logs, plots,
tables, notebooks, and batch cards remain uncommitted unless the user
explicitly authorizes a specific exception. Frequent read-only Git inspection
and focused core-code checkpoint commits remain required.

The user then clarified that durable project knowledge must also be versioned
and published. The active law now requires tracking, committing, and pushing
core `RecGsfTracking` C++ source/header changes together with coherent
documentation changes, `AGENTS.md`, `.agents/` maintenance content, and
durable `agents_record/` status/history records. Run cards/options, analysis
scripts, build files, and generated experiment products remain excluded unless
the user explicitly authorizes a specific exception.

`DumpGsfTrks/gsf.py.bk` is the standing exception to the run-card exclusion.
The user had already designated it as the maintained complete-property
comparison card, so it must be versioned with the package option reference;
otherwise the committed documentation and remote card would contradict one
another.
