#!/usr/bin/env python3
"""Relate topology-clean GSF outcomes to dominant Geant4 loss surfaces."""

from __future__ import annotations

import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path
import re


TRANSITION_BINS = [(0, 3), (3, 5), (5, 7), (7, 9), (9, 12),
                   (12, 50), (50, 100), (100, 150), (150, 200), (200, 10000)]
LOSS_BINS = [(0, 1), (1, 3), (3, 5), (5, 7), (7, 10), (10, 101)]


def bin_label(index: int) -> str:
    for low, high in TRANSITION_BINS:
        if low <= index < high:
            return f"{low}-{high - 1}"
    return "none"


def loss_bin_label(loss: float) -> str:
    for low, high in LOSS_BINS:
        if low <= loss < high:
            return f"{low}-{high}"
    return "none"


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--outcomes", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/topology_clean_event_outcomes.csv"))
    parser.add_argument(
        "--transitions", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/full_500_sample_2026-07-12/surface_owned_transitions.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13"))
    return parser.parse_args()


def main() -> None:
    args = arguments()
    events = {}
    with args.outcomes.open(newline="") as stream:
        for row in csv.DictReader(stream):
            if row["category"] == "light_ebrem" and not int(
                    row["excluded_secondary_topology"]):
                events[(int(row["seed"]), int(row["entry"]))] = row

    dominant: dict[tuple[int, int], dict[str, float | int]] = {}
    seed_pattern = re.compile(r"-([0-9]+)\.root$")
    with args.transitions.open(newline="") as stream:
        for row in csv.DictReader(stream):
            if int(row["n_ebrem_steps"] or 0) <= 0:
                continue
            match = seed_pattern.search(row["source_file"])
            if not match:
                continue
            key = (int(match.group(1)), int(row["event_id"]))
            if key not in events:
                continue
            loss = float(row["loss_GeV"])
            if (key not in dominant
                    or loss > dominant[key]["dominant_transition_loss_GeV"]):
                dominant[key] = {
                    "dominant_transition_index": int(row["transition_index"]),
                    "dominant_transition_loss_pct": 100 * (1 - float(row["z"])),
                    "dominant_transition_loss_GeV": loss,
                    "dominant_transition_g4_tX0": float(row["g4_t_over_x0"]),
                }

    joined = []
    for key, event in sorted(events.items()):
        loss = dominant.get(key)
        if loss is None:
            continue
        joined.append({
            "seed": key[0], "entry": key[1], "outcome": event["outcome"],
            "owned_loss_pct": event["owned_loss_pct"],
            "lcio_residual_pct": event["lcio_residual_pct"],
            "gsf_residual_pct": event["gsf_residual_pct"],
            **loss,
            "dominant_transition_bin": bin_label(
                int(loss["dominant_transition_index"])),
            "dominant_loss_bin_pct": loss_bin_label(
                float(loss["dominant_transition_loss_pct"])),
        })
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output = args.output_dir / "light_outcomes_by_dominant_loss_surface.csv"
    with output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(joined[0]))
        writer.writeheader()
        writer.writerows(joined)

    counts = defaultdict(Counter)
    for row in joined:
        counts[row["dominant_transition_bin"]][row["outcome"]] += 1
    outcomes = sorted({row["outcome"] for row in joined})
    summary = args.output_dir / "light_outcomes_by_dominant_loss_surface_summary.csv"
    with summary.open("w", newline="") as stream:
        fields = ["dominant_transition_bin", "count", *outcomes]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for low, high in TRANSITION_BINS:
            label = f"{low}-{high - 1}"
            counter = counts[label]
            writer.writerow({"dominant_transition_bin": label,
                             "count": sum(counter.values()),
                             **{outcome: counter[outcome] for outcome in outcomes}})
    print(f"joined {len(joined)}/{len(events)} topology-clean light events")
    print(summary.read_text(), end="")

    # Control the surface comparison for dominant loss size.  Recovery-eligible
    # outcomes all start from an LCIO residual below -1%; truth-like outcomes
    # are intentionally omitted from the recovery fraction denominator.
    recovery_outcomes = {"good_recovery", "near_recovery", "partial_recovery",
                         "overshoot", "missed_recovery"}
    cross = defaultdict(Counter)
    for row in joined:
        if row["outcome"] in recovery_outcomes:
            cross[(row["dominant_transition_bin"],
                   row["dominant_loss_bin_pct"])][row["outcome"]] += 1
    cross_output = args.output_dir / "light_recovery_by_loss_and_surface.csv"
    with cross_output.open("w", newline="") as stream:
        fields = ["dominant_transition_bin", "dominant_loss_bin_pct", "count",
                  "recovered_count", "recovered_fraction",
                  *sorted(recovery_outcomes)]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for transition_low, transition_high in TRANSITION_BINS:
            transition_label = f"{transition_low}-{transition_high - 1}"
            for loss_low, loss_high in LOSS_BINS:
                loss_label = f"{loss_low}-{loss_high}"
                counter = cross[(transition_label, loss_label)]
                count = sum(counter.values())
                recovered = count - counter["missed_recovery"]
                writer.writerow({
                    "dominant_transition_bin": transition_label,
                    "dominant_loss_bin_pct": loss_label,
                    "count": count, "recovered_count": recovered,
                    "recovered_fraction": recovered / count if count else "",
                    **{outcome: counter[outcome]
                       for outcome in sorted(recovery_outcomes)},
                })

    # Produce reproducible, comparable-loss missed/good pairs across surfaces.
    missed = [row for row in joined if row["outcome"] == "missed_recovery"]
    good = [row for row in joined if row["outcome"] == "good_recovery"]
    pairs = []
    used_good = set()
    for left in sorted(missed, key=lambda row: (
            int(row["dominant_transition_index"]),
            float(row["dominant_transition_loss_pct"]))):
        candidates = [(abs(float(left["dominant_transition_loss_pct"]) -
                           float(right["dominant_transition_loss_pct"])), right)
                      for right in good
                      if (right["seed"], right["entry"]) not in used_good and
                      int(right["dominant_transition_index"]) >
                      int(left["dominant_transition_index"])]
        if not candidates:
            continue
        difference, right = min(candidates, key=lambda item: item[0])
        if difference > 0.5:
            continue
        used_good.add((right["seed"], right["entry"]))
        pairs.append({
            "missed_seed": left["seed"], "missed_entry": left["entry"],
            "missed_transition": left["dominant_transition_index"],
            "missed_loss_pct": left["dominant_transition_loss_pct"],
            "missed_lcio_residual_pct": left["lcio_residual_pct"],
            "missed_gsf_residual_pct": left["gsf_residual_pct"],
            "good_seed": right["seed"], "good_entry": right["entry"],
            "good_transition": right["dominant_transition_index"],
            "good_loss_pct": right["dominant_transition_loss_pct"],
            "good_lcio_residual_pct": right["lcio_residual_pct"],
            "good_gsf_residual_pct": right["gsf_residual_pct"],
            "absolute_loss_difference_pct": difference,
        })
    pair_output = args.output_dir / "comparable_loss_missed_good_pairs.csv"
    with pair_output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(pairs[0]) if pairs else [])
        if pairs:
            writer.writeheader()
            writer.writerows(pairs)
    print(f"wrote {len(pairs)} comparable-loss missed/good surface pairs")


if __name__ == "__main__":
    main()
