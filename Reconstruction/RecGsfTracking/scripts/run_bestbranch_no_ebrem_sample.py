#!/usr/bin/env python3
"""Rerun one Geant4 tracker-eBrem category with reverse BestBranch output."""

from __future__ import annotations

import argparse
import csv
from concurrent.futures import ThreadPoolExecutor, as_completed
import os
from pathlib import Path
import subprocess


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--categories",
        default="TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/"
                "plots/gsf_reverse_vs_lcio_pt_resolution_by_tracker_ebrem_events.csv",
        type=Path)
    parser.add_argument(
        "--category", default="no_ebrem",
        choices=("no_ebrem", "light_ebrem", "hard_ebrem"))
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--jobs", default=4, type=int)
    args = parser.parse_args()

    selected: dict[int, list[int]] = {}
    with args.categories.open() as source:
        for row in csv.DictReader(source):
            if row["category"] == args.category:
                selected.setdefault(int(row["seed"]), []).append(int(row["entry"]))
    if args.output_dir is None:
        args.output_dir = Path(f"/tmp/gsf-bestbranch-{args.category.replace('_', '-')}")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    expected = sum(map(len, selected.values()))

    option = ("Reconstruction/RecGsfTracking/options/"
              "run_gsf_reverse_template.py")
    runner = "build.105.0.0.x86_64-el9-gcc11-opt/run"

    def run_seed(seed: int, entries: list[int]) -> tuple[int, int]:
        env = os.environ.copy()
        env.update({
            "GSF_EVTMAX": "10",
            "GSF_INPUT_FILE": f"trk-e--2.0-85-{seed}.root",
            "GSF_EDM_OUTPUT": str(args.output_dir / f"gsf-best-{seed}.root"),
            "GSF_TUPLE_OUTPUT": str(args.output_dir / f"gsf-flat-best-{seed}.root"),
            "GSF_SELECTED_EVENT_INDICES": ",".join(map(str, entries)),
            "GSF_BH_MODEL": "CEPC2GeV85StepConditioned",
            "GSF_MATERIAL_PATH_MODE": "DD4hepBetweenSurfaces",
            "GSF_REVERSE_OUTPUT_MODE": "BestBranch",
        })
        log_path = args.output_dir / f"seed-{seed}.log"
        with log_path.open("w") as log:
            result = subprocess.run(
                [runner, "gaudirun.py", option], env=env,
                stdout=log, stderr=subprocess.STDOUT, check=False)
        # Gaudi returns 4 after its normal input-exhaustion ScheduledStop.
        if result.returncode not in (0, 4):
            raise RuntimeError(f"seed {seed} failed with {result.returncode}; see {log_path}")
        return seed, len(entries)

    completed = 0
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_seed, seed, entries)
                   for seed, entries in sorted(selected.items())]
        for future in as_completed(futures):
            seed, count = future.result()
            completed += count
            print(f"seed {seed:3d}: {count} selected; total {completed}/{expected}", flush=True)
    print(f"completed {completed} {args.category} events in {len(selected)} files")


if __name__ == "__main__":
    main()
