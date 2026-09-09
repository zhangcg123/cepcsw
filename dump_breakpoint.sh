#!/bin/bash
# One frozen per-job manifest, prepared by subbreakpointjobs.sh.
if [ "$#" -ne 1 ] || [ ! -f "$1" ]; then
    echo "Usage: $0 /absolute/path/to/job.json" >&2
    exit 2
fi
manifest=$(realpath -- "$1") || exit 1
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
cd -- "${script_dir}" || exit 1
# Source the existing environment before nounset: external setup uses unset vars.
source setup.sh || exit 1
#set -eu
export OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1 MKL_NUM_THREADS=1
export BLIS_NUM_THREADS=1 NUMEXPR_NUM_THREADS=1
exec python3 Reconstruction/RecBreakpoint/options/batch_breakpoint.py run "${manifest}"
