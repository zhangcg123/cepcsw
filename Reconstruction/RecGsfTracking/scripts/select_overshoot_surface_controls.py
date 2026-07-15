#!/usr/bin/env python3
"""Select same-surface, comparable-loss controls for light-eBrem overshoots."""

import argparse
import csv
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/light_outcomes_by_dominant_loss_surface.csv"))
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    rows = list(csv.DictReader(args.input.open(newline="")))
    actionable = [row for row in rows
                  if 5 <= int(row["dominant_transition_index"]) <= 11]
    overshoots = [row for row in actionable if row["outcome"] == "overshoot"]
    controls = [row for row in actionable
                if row["outcome"] in {"good_recovery", "partial_recovery",
                                      "near_recovery"}]
    output = []
    for over in overshoots:
        transition = int(over["dominant_transition_index"])
        loss = float(over["dominant_transition_loss_pct"])
        same_surface = [row for row in controls
                        if int(row["dominant_transition_index"]) == transition]
        if not same_surface:
            continue
        control = min(same_surface, key=lambda row:
                      abs(float(row["dominant_transition_loss_pct"]) - loss))
        output.append({
            "overshoot_seed": over["seed"],
            "overshoot_entry": over["entry"],
            "transition": transition,
            "overshoot_dominant_loss_pct": loss,
            "overshoot_owned_loss_pct": over["owned_loss_pct"],
            "overshoot_lcio_residual_pct": over["lcio_residual_pct"],
            "overshoot_gsf_residual_pct": over["gsf_residual_pct"],
            "control_outcome": control["outcome"],
            "control_seed": control["seed"],
            "control_entry": control["entry"],
            "control_dominant_loss_pct": control["dominant_transition_loss_pct"],
            "control_owned_loss_pct": control["owned_loss_pct"],
            "control_lcio_residual_pct": control["lcio_residual_pct"],
            "control_gsf_residual_pct": control["gsf_residual_pct"],
            "absolute_dominant_loss_difference_pct": abs(
                float(control["dominant_transition_loss_pct"]) - loss),
        })
    output.sort(key=lambda row: (row["transition"],
                                -abs(float(row["overshoot_gsf_residual_pct"]))))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(output[0]))
        writer.writeheader()
        writer.writerows(output)
    print(f"wrote {len(output)} overshoot/control pairs to {args.output}")
    for row in output:
        print("%(overshoot_seed)s/%(overshoot_entry)s tr=%(transition)s "
              "loss=%(overshoot_dominant_loss_pct).3f gsf=%(overshoot_gsf_residual_pct)s "
              "vs %(control_seed)s/%(control_entry)s %(control_outcome)s "
              "loss=%(control_dominant_loss_pct)s gsf=%(control_gsf_residual_pct)s "
              "dloss=%(absolute_dominant_loss_difference_pct).3f" % row)


if __name__ == "__main__":
    main()
