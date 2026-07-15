#!/usr/bin/env python3
"""Select unique same-transition, nearest-loss controls for overshoots."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    base = Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85")
    parser.add_argument("--overshoots", type=Path, default=base /
                        "surveys/topology_clean_2026-07-13/maxcomp12_transition_7_8_overshoot_0p5_2pct_all57.csv")
    parser.add_argument("--population", type=Path, default=base /
                        "plots/maxcomp12_new_tuples_2026-07-14/transition_location/transition_location_event_residuals.csv")
    parser.add_argument("--control-max-abs-residual", type=float, default=0.5)
    parser.add_argument("--output", type=Path, default=base /
                        "surveys/topology_clean_2026-07-13/maxcomp12_transition_7_8_overshoot_loss_matched_controls57.csv")
    args = parser.parse_args()

    with args.overshoots.open(newline="") as stream:
        overshoots = list(csv.DictReader(stream))
    with args.population.open(newline="") as stream:
        controls = [row for row in csv.DictReader(stream)
                    if row["transition_group"] == "7-8"
                    and abs(float(row["gsf_residual_pct"]))
                    <= args.control_max_abs_residual]
    used = set()
    selected = []
    for overshoot in sorted(
            overshoots, key=lambda row: (float(row["owned_loss_pct"]),
                                         int(row["seed"]), int(row["entry"]))):
        truth_transition = int(overshoot["dominant_transition_index"])
        loss = float(overshoot["owned_loss_pct"])
        available = [row for row in controls
                     if (int(row["seed"]), int(row["entry"])) not in used
                     and int(row["dominant_transition_index"]) == truth_transition]
        if not available:
            raise RuntimeError(f"No unused transition-{truth_transition} control")
        control = min(available, key=lambda row: (
            abs(float(row["owned_loss_pct"]) - loss),
            int(row["seed"]), int(row["entry"])))
        event_id = (int(control["seed"]), int(control["entry"]))
        used.add(event_id)
        selected.append({
            "seed": event_id[0], "entry": event_id[1],
            "matched_overshoot_seed": overshoot["seed"],
            "matched_overshoot_entry": overshoot["entry"],
            "dominant_transition_index": control["dominant_transition_index"],
            "owned_loss_pct": control["owned_loss_pct"],
            "matched_overshoot_owned_loss_pct": overshoot["owned_loss_pct"],
            "absolute_loss_difference_pct": abs(
                float(control["owned_loss_pct"]) - loss),
            "lcio_residual_pct": control["lcio_residual_pct"],
            "gsf_residual_pct": control["gsf_residual_pct"],
        })
    selected.sort(key=lambda row: (row["seed"], row["entry"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(selected[0]))
        writer.writeheader()
        writer.writerows(selected)
    print(f"Selected {len(selected)} unique controls")
    print(args.output)


if __name__ == "__main__":
    main()
