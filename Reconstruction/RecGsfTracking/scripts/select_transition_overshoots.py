#!/usr/bin/env python3
"""Select deterministic transition-bin GSF overshoots and list their pT."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    base = Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots")
    parser.add_argument("--transitions", type=Path, default=base /
                        "maxcomp12_new_tuples_2026-07-14/transition_location/transition_location_event_residuals.csv")
    parser.add_argument("--tuples", type=Path, default=base /
                        "maxcomp12_new_tuples_2026-07-14/matched_event_residuals.csv")
    parser.add_argument("--transition-group", default="7-8")
    parser.add_argument("--min-residual", type=float, default=0.5)
    parser.add_argument("--max-residual", type=float, default=2.0)
    parser.add_argument("--count", type=int, default=50)
    parser.add_argument("--output", type=Path, default=base /
                        "maxcomp12_new_tuples_2026-07-14/transition_location/transition_7_8_overshoot_0p5_2pct_first50.csv")
    args = parser.parse_args()

    with args.tuples.open(newline="") as stream:
        tuples = {(int(row["seed"]), int(row["entry"])): row
                  for row in csv.DictReader(stream)}
    candidates = []
    with args.transitions.open(newline="") as stream:
        for row in csv.DictReader(stream):
            residual = float(row["gsf_residual_pct"])
            if (row["transition_group"] == args.transition_group
                    and args.min_residual <= residual <= args.max_residual):
                candidates.append(row)
    candidates.sort(key=lambda row: (int(row["seed"]), int(row["entry"])))

    selected = []
    for rank, row in enumerate(candidates[:args.count], 1):
        event_id = (int(row["seed"]), int(row["entry"]))
        source = tuples[event_id]
        selected.append({
            "selection_rank": rank, "seed": event_id[0], "entry": event_id[1],
            "transition_group": row["transition_group"],
            "dominant_transition_index": row["dominant_transition_index"],
            "dominant_transition_loss_pct": row["dominant_transition_loss_pct"],
            "owned_loss_pct": row["owned_loss_pct"],
            "truth_pT_GeV": source["mc_pT"],
            "lcio_pT_GeV": source["lcio_pT"],
            "gsf_maxcomp12_pT_GeV": source["gsf_pT"],
            "lcio_residual_pct": row["lcio_residual_pct"],
            "gsf_maxcomp12_residual_pct": row["gsf_residual_pct"],
        })
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(selected[0]))
        writer.writeheader()
        writer.writerows(selected)
    print(f"Candidates: {len(candidates)}; selected: {len(selected)}")
    print(args.output)


if __name__ == "__main__":
    main()
