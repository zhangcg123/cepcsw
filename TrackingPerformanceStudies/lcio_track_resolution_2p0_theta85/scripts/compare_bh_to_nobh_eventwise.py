#!/usr/bin/env python3
"""Compare BH BestBranch and no-BH reverse outputs event by event."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import ROOT


def read_pt(directory: Path, seed: int, entry: int) -> tuple[float, float]:
    source = ROOT.TFile.Open(str(directory / f"gsf-flat-best-{seed}.root"))
    if not source or source.IsZombie():
        raise RuntimeError(f"cannot open seed {seed} in {directory}")
    tree = source.Get("gsf_tuple")
    tree.GetEntry(entry)
    result = float(tree.mc_pT), float(tree.gsf_pT)
    source.Close()
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--events", type=Path, required=True)
    parser.add_argument("--category", default="no_ebrem")
    parser.add_argument("--bh-dir", type=Path, required=True)
    parser.add_argument("--nobh-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    selected = []
    with args.events.open() as source:
        for row in csv.DictReader(source):
            if row["category"] == args.category:
                selected.append(row)

    output_rows = []
    for row in selected:
        seed, entry = int(row["seed"]), int(row["entry"])
        truth, bh_pt = read_pt(args.bh_dir, seed, entry)
        nobh_truth, nobh_pt = read_pt(args.nobh_dir, seed, entry)
        if abs(truth - nobh_truth) > 1e-9:
            raise RuntimeError(f"truth mismatch for seed {seed}, entry {entry}")
        bh_residual = 100.0 * (bh_pt - truth) / truth
        nobh_residual = 100.0 * (nobh_pt - truth) / truth
        output_rows.append({
            "seed": seed, "entry": entry, "truth_pt_GeV": truth,
            "bh_pt_GeV": bh_pt, "nobh_pt_GeV": nobh_pt,
            "bh_residual_pct": bh_residual,
            "nobh_residual_pct": nobh_residual,
            "bh_minus_nobh_pct": bh_residual - nobh_residual,
        })

    output_rows.sort(key=lambda row: abs(row["bh_minus_nobh_pct"]), reverse=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as target:
        writer = csv.DictWriter(target, fieldnames=output_rows[0].keys())
        writer.writeheader()
        writer.writerows(output_rows)

    for threshold in (0.1, 0.5, 1.0, 5.0):
        count = sum(abs(row["bh_minus_nobh_pct"]) > threshold for row in output_rows)
        print(f"|BH-noBH| > {threshold:g}%: {count}/{len(output_rows)}")
    print("largest changes:")
    for row in output_rows[:15]:
        print(row)


if __name__ == "__main__":
    main()
