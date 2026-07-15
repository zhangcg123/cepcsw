#!/usr/bin/env python3
"""Compare the fixed random-light100 old-24 result with a new tuple directory."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import numpy as np
import ROOT


def metrics(values: np.ndarray) -> dict[str, float | int]:
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    return {
        "count": len(values),
        "median_pct": median,
        "q16_pct": q16,
        "q84_pct": q84,
        "width68_pct": 0.5 * (q84 - q16),
        "mean_abs_pct": np.mean(np.abs(values)),
        "rms_pct": np.sqrt(np.mean(values * values)),
        "inside_1pct": np.count_nonzero(np.abs(values) <= 1.0),
        "inside_2pct": np.count_nonzero(np.abs(values) <= 2.0),
        "inside_5pct": np.count_nonzero(np.abs(values) <= 5.0),
        "inside_10pct": np.count_nonzero(np.abs(values) <= 10.0),
    }


def tuple_row(path: Path, entry: int) -> dict[str, float | int]:
    source = ROOT.TFile.Open(str(path))
    tree = source.Get("gsf_tuple") if source else None
    if not tree:
        raise RuntimeError(f"missing gsf_tuple in {path}")
    try:
        for row in tree:
            if int(row.iev) == entry + 1:
                return {
                    "truth_pt": float(row.mc_pT),
                    "lcio_pt": float(row.lcio_pT),
                    "new_pt": float(row.gsf_pT),
                    "new_nhits": int(row.gsf_nhits),
                }
    finally:
        source.Close()
    raise RuntimeError(f"event {entry} absent from {path}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sample", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()

    with args.sample.open(newline="") as stream:
        sample = {(int(row["seed"]), int(row["entry"])): row
                  for row in csv.DictReader(stream)}
    with args.baseline.open(newline="") as stream:
        baseline = {(int(row["seed"]), int(row["entry"])): row
                    for row in csv.DictReader(stream)}
    if sample.keys() != baseline.keys():
        raise RuntimeError("sample and baseline event sets differ")

    rows = []
    for key in sorted(sample):
        seed, entry = key
        old = baseline[key]
        new = tuple_row(args.candidate_dir / f"gsf-flat-{seed}.root", entry)
        truth = float(new["truth_pt"])
        old_pt = float(old["candidate_pt"])
        old_res = 100.0 * (old_pt - truth) / truth
        new_res = 100.0 * (float(new["new_pt"]) - truth) / truth
        lcio_res = 100.0 * (float(new["lcio_pt"]) - truth) / truth
        abs_gain = abs(old_res) - abs(new_res)
        rows.append({
            "seed": seed,
            "entry": entry,
            "outcome": sample[key]["outcome"],
            "owned_loss_pct": sample[key]["owned_loss_pct"],
            "truth_pt": truth,
            "lcio_pt": new["lcio_pt"],
            "old_pt": old_pt,
            "new_pt": new["new_pt"],
            "lcio_residual_pct": lcio_res,
            "old_residual_pct": old_res,
            "new_residual_pct": new_res,
            "new_minus_old_pct": new_res - old_res,
            "absolute_error_improvement_pct": abs_gain,
            "material_change": int(abs(new_res - old_res) > 0.1),
            "improved": int(abs_gain > 0.1),
            "worsened": int(abs_gain < -0.1),
            "old_nhits": int(old["candidate_nhits"]),
            "new_nhits": new["new_nhits"],
            "hit_mismatch": int(int(old["candidate_nhits"]) !=
                                int(new["new_nhits"])),
        })

    args.output_dir.mkdir(parents=True, exist_ok=True)
    eventwise = args.output_dir / "random_light100_old24_vs_posterior24_eventwise.csv"
    with eventwise.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    old_values = np.asarray([float(row["old_residual_pct"]) for row in rows])
    new_values = np.asarray([float(row["new_residual_pct"]) for row in rows])
    summaries = [
        {"sample": "old24", **metrics(old_values)},
        {"sample": "posterior_order24", **metrics(new_values)},
    ]
    summary = args.output_dir / "random_light100_old24_vs_posterior24_summary.csv"
    with summary.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(summaries[0]))
        writer.writeheader()
        writer.writerows(summaries)

    good_lcio = [row for row in rows if abs(float(row["lcio_residual_pct"])) <= 0.5]
    changed = [row for row in rows if row["material_change"]]
    print(summary.read_text(), end="")
    print(f"materially_changed={len(changed)} "
          f"improved={sum(row['improved'] for row in rows)} "
          f"worsened={sum(row['worsened'] for row in rows)}")
    print(f"hit_mismatches={sum(row['hit_mismatch'] for row in rows)}")
    print(f"good_lcio={len(good_lcio)} "
          f"old_2to4={sum(2 <= float(row['old_residual_pct']) <= 4 for row in good_lcio)} "
          f"new_2to4={sum(2 <= float(row['new_residual_pct']) <= 4 for row in good_lcio)}")
    for row in sorted(changed, key=lambda item: abs(float(item["new_minus_old_pct"])),
                      reverse=True):
        print(f"{row['seed']}/{row['entry']} {row['outcome']} "
              f"LCIO={float(row['lcio_pt']):.5f} OLD={float(row['old_pt']):.5f} "
              f"NEW={float(row['new_pt']):.5f} "
              f"delta={float(row['new_minus_old_pct']):+.4f}pp")
    print(eventwise)


if __name__ == "__main__":
    main()
