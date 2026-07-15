#!/usr/bin/env python3
"""Rerun a topology-clean category with a chosen reverse selection mode."""

from __future__ import annotations

import argparse
import csv
from concurrent.futures import ThreadPoolExecutor, as_completed
import os
from pathlib import Path
import subprocess


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--outcomes", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/topology_clean_event_outcomes.csv"))
    parser.add_argument("--category", required=True,
                        choices=("no_ebrem", "light_ebrem", "hard_ebrem"))
    parser.add_argument("--selection-mode", default="DominantLineage",
                        choices=("AggregateWeight", "DominantLineage",
                                 "SurfaceConsistency"))
    parser.add_argument("--surface-consistency-uninformative-floor", type=float,
                        default=0.05)
    parser.add_argument("--max-components", type=int, default=24)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument(
        "--event-list", type=Path,
        help="Optional CSV with seed and entry columns; bypass category selection")
    parser.add_argument(
        "--event-list-filter-column",
        help="With --event-list, retain only rows whose named column is nonzero")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--input-dir", type=Path, default=Path("tuples285"))
    parser.add_argument("--input-pattern", default="trk-e--2.0-85-{seed}.root",
                        help="Filename pattern below --input-dir")
    parser.add_argument("--workflow", choices=("reverse", "smoother", "cms"),
                        default="reverse")
    parser.add_argument("--resume", action="store_true",
                        help="Skip seeds with a tuple and successful completion log")
    parser.add_argument("--verbose-components", action="store_true")
    parser.add_argument("--counterfactual-loss-scan", action="store_true")
    parser.add_argument(
        "--counterfactual-loss-fractions",
        default="0.04,0.05,0.06,0.07,0.08,0.09,0.10,0.12")
    parser.add_argument("--counterfactual-loss-variance", type=float,
                        default=2.0e-4)
    return parser.parse_args()


def main() -> None:
    args = arguments()
    selected: dict[int, list[int]] = {}
    truth_transitions: dict[int, dict[int, int]] = {}
    source = args.event_list or args.outcomes
    with source.open(newline="") as stream:
        for row in csv.DictReader(stream):
            event_list_selected = args.event_list and (
                not args.event_list_filter_column or
                int(row[args.event_list_filter_column]))
            if event_list_selected or (not args.event_list and
                    row["category"] == args.category
                                   and not int(row["excluded_secondary_topology"])):
                selected.setdefault(int(row["seed"]), []).append(int(row["entry"]))
                if args.counterfactual_loss_scan:
                    if "dominant_transition_index" not in row:
                        raise RuntimeError(
                            "counterfactual scan requires dominant_transition_index")
                    truth_transitions.setdefault(int(row["seed"]), {})[
                        int(row["entry"])] = int(row["dominant_transition_index"])
    args.output_dir.mkdir(parents=True, exist_ok=True)
    option = "Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py"
    runner = "build.105.0.0.x86_64-el9-gcc11-opt/run"
    expected = sum(map(len, selected.values()))

    def seed_complete(seed: int) -> bool:
        tuple_path = args.output_dir / f"gsf-flat-{seed}.root"
        log_path = args.output_dir / f"seed-{seed}.log"
        if not tuple_path.exists() or not log_path.exists():
            return False
        return "Application Manager Terminated successfully" in log_path.read_text(
            errors="replace")

    def run_seed(seed: int, entries: list[int]) -> tuple[int, int]:
        env = os.environ.copy()
        env.update({
            "GSF_EVTMAX": "10",
            "GSF_INPUT_FILE": str(
                args.input_dir / args.input_pattern.format(seed=seed)),
            "GSF_EDM_OUTPUT": str(args.output_dir / f"gsf-{seed}.root"),
            "GSF_TUPLE_OUTPUT": str(args.output_dir / f"gsf-flat-{seed}.root"),
            "GSF_SELECTED_EVENT_INDICES": ",".join(map(str, entries)),
            "GSF_REVERSE_SELECTION_MODE": args.selection_mode,
            "GSF_MAX_COMPONENTS": str(args.max_components),
            "GSF_REVERSE_FILTERING": "1" if args.workflow == "reverse" else "0",
            "GSF_GAUSSIAN_SUM_SMOOTHING": (
                "1" if args.workflow == "smoother" else "0"),
            "GSF_CMS_GSF_SMOOTHING": "1" if args.workflow == "cms" else "0",
            "GSF_OUTPUT_MODE": (
                "WeightedMean" if args.workflow == "smoother" else "BestBranch"),
            "GSF_SURFACE_CONSISTENCY_UNINFORMATIVE_FLOOR": str(
                args.surface_consistency_uninformative_floor),
            "GSF_VERBOSE_COMPONENTS": "1" if args.verbose_components else "0",
            "GSF_COUNTERFACTUAL_LOSS_SCAN": (
                "1" if args.counterfactual_loss_scan else "0"),
            "GSF_COUNTERFACTUAL_TRUTH_TRANSITION_MAP": ",".join(
                f"{entry}:{transition}" for entry, transition in
                sorted(truth_transitions.get(seed, {}).items())),
            "GSF_COUNTERFACTUAL_LOSS_FRACTIONS":
                args.counterfactual_loss_fractions,
            "GSF_COUNTERFACTUAL_LOSS_VARIANCE": str(
                args.counterfactual_loss_variance),
        })
        log_path = args.output_dir / f"seed-{seed}.log"
        with log_path.open("w") as log:
            result = subprocess.run(
                [runner, "gaudirun.py", option], env=env,
                stdout=log, stderr=subprocess.STDOUT, check=False)
        if result.returncode not in (0, 4):
            raise RuntimeError(
                f"seed {seed} failed with {result.returncode}; see {log_path}")
        return seed, len(entries)

    skipped = {seed for seed in selected if args.resume and seed_complete(seed)}
    completed = sum(len(selected[seed]) for seed in skipped)
    if skipped:
        print(f"resuming after {len(skipped)} seeds / {completed} events", flush=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_seed, seed, entries)
                   for seed, entries in sorted(selected.items())
                   if seed not in skipped]
        for future in as_completed(futures):
            seed, count = future.result()
            completed += count
            print(f"seed {seed:3d}: {count} selected; total {completed}/{expected}",
                  flush=True)
    print(f"completed {completed} {args.category} events in {len(selected)} files")


if __name__ == "__main__":
    main()
