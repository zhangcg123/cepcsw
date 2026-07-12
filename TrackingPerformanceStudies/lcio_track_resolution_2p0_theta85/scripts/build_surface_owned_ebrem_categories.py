#!/usr/bin/env python3
"""Categorize matched events using owned Geant4 surface transitions."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path
import re

import ROOT


SEED_RE = re.compile(r"-(\d+)\.root$")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("transitions", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--hard-threshold", type=float, default=0.10)
    args = parser.parse_args()

    events = {}
    with args.transitions.open() as source:
        for row in csv.DictReader(source):
            match = SEED_RE.search(row["source_file"])
            if not match:
                raise RuntimeError(f"Cannot extract seed from {row['source_file']}")
            key = (int(match.group(1)), int(row["event_id"]))
            event = events.setdefault(key, {
                "transition_count": 0, "ebrem_transition_count": 0,
                "ebrem_step_count": 0, "max_frac": 0.0,
                "retained_product": 1.0, "sum_loss_GeV": 0.0,
            })
            event["transition_count"] += 1
            steps = int(row["n_ebrem_steps"])
            if steps <= 0:
                continue
            p_before = float(row["p_before_GeV"])
            loss = max(0.0, float(row["ebrem_step_loss_sum_GeV"]))
            fraction = min(1.0, loss / p_before) if p_before > 0.0 else 0.0
            event["ebrem_transition_count"] += 1
            event["ebrem_step_count"] += steps
            event["max_frac"] = max(event["max_frac"], fraction)
            event["retained_product"] *= 1.0 - fraction
            event["sum_loss_GeV"] += loss

    rows = []
    flat_files = {}
    for (seed, entry), event in sorted(events.items()):
        cumulative = 1.0 - event.pop("retained_product")
        if event["ebrem_step_count"] == 0:
            category = "no_ebrem"
        elif event["max_frac"] >= args.hard_threshold or cumulative >= args.hard_threshold:
            category = "hard_ebrem"
        else:
            category = "light_ebrem"
        if seed not in flat_files:
            root_file = ROOT.TFile.Open(f"gsf_flat-e--2.0-85-{seed}.root")
            if not root_file or root_file.IsZombie():
                raise RuntimeError(f"Cannot open flat tuple for seed {seed}")
            flat_files[seed] = (root_file, root_file.Get("gsf_tuple"))
        tree = flat_files[seed][1]
        tree.GetEntry(entry)
        truth = float(tree.mc_pT)
        lcio = float(tree.lcio_pT)
        gsf = float(tree.gsf_pT)
        rows.append({
            "seed": seed, "entry": entry, "category": category,
            **event, "cumulative_frac": cumulative,
            "lcio_residual_pct": 100.0 * (lcio - truth) / truth,
            "gsf_residual_pct": 100.0 * (gsf - truth) / truth,
        })
    for root_file, _ in flat_files.values():
        root_file.Close()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)
    counts = {category: sum(row["category"] == category for row in rows)
              for category in ("no_ebrem", "light_ebrem", "hard_ebrem")}
    print(f"wrote {len(rows)} events to {args.output}")
    print("counts", counts)


if __name__ == "__main__":
    main()
