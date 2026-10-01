# Retained simulation samples; removed other generated tuples

The user explicitly requested retaining only `sim_large_barrel_20260823/`
and `sim_large_endcap_20260823/` as tuple samples and removing all other
tuples. Before deletion, a workspace scan found 1,534 other ROOT files with
183.942 GiB logical size. ROOT key inspection identified 1,525 readable
tuple files, five damaged/empty files bearing tuple filenames, and four
histogram-only ROOT plot artifacts. The former 1,530 files were removed.
The four histogram files under G4MaterialStepComparison were preserved because
they are plots, not tuples. The loose root-level `gsf-reverse-e--2.0-85-1.root`
and 89 other temporary RecBreakpoint ROOT outputs in six explicitly inspected
`/tmp/recbreakpoint*` diagnostic directories were also removed. Non-ROOT
scripts, run cards, logs, plots, CSV/JSON summaries, documentation, and source
files were left in place. Generated ROOT tuples were untracked by Git and are
not recoverable from the repository; they could be recreated only from the
retained simulation samples and the relevant workflow, where applicable.

Read-only post-deletion inventory: the only remaining tuple files in the
workspace are 251 barrel simulation files (251,000 events; 658.782 GiB logical)
and 500 endcap simulation files (100,000 events; 260.258 GiB logical). Four
histogram-only ROOT plot files remain. Filesystem free space increased, though
the reported filesystem usage need not equal summed logical file sizes.

Consequences: historical GSF and breakpoint summary tables and dated records
remain interpretable, but their raw ROOT outputs no longer exist. The
2,000-event shared-sigma study and the later 50-event default-on breakdown
study are preserved in CSV/JSON and dated records, not rerunnable directly
from their deleted tracker tuples. The installed RecBreakpoint library still
lacks the new default-on free/truth breakdown fields; the verified rebuilt
module is checkpoint b26d0b3. To collect new such tuples, install the verified
module when no jobs use the old installed version and regenerate tracking
inputs from retained simulation tuples. No source, card, or remote changes
were made by this cleanup.
