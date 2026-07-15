#!/usr/bin/env python3
"""Build a reproducible topology-clean GSF outcome catalogue."""

from __future__ import annotations

import argparse
import csv
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np


LOSS_BINS = [(0, 1), (1, 3), (3, 5), (5, 7), (7, 10)]
RESIDUAL_BINS = [(0, 1), (1, 3), (3, 5), (5, 10), (10, float("inf"))]


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--events", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/full_500_sample_2026-07-12/surface_owned_ebrem_event_categories.csv"))
    parser.add_argument(
        "--topology", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13/secondary_tracker_activity_event_ids.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13"))
    return parser.parse_args()


def interval_label(value: float, bins) -> str:
    for low, high in bins:
        if low <= value < high:
            upper = "inf" if np.isinf(high) else f"{high:g}"
            return f"{low:g}-{upper}"
    return "outside"


def classify(category: str, loss: float, lcio: float, gsf: float) -> tuple[str, float]:
    correction = np.nan
    if lcio < -1:
        correction = (gsf - lcio) / (-lcio)

    if abs(lcio) <= 1:
        if abs(gsf) <= 1:
            return "truth_like_lcio_preserved", correction
        if abs(gsf) > abs(lcio) + 0.5:
            return "truth_like_lcio_degraded", correction
        return "truth_like_lcio_other", correction

    if lcio < -1:
        improvement = abs(lcio) - abs(gsf)
        if abs(gsf) <= 1:
            return "good_recovery", correction
        if improvement < -0.1:
            return "degradation", correction
        if correction < 0.2:
            return "missed_recovery", correction
        if correction < 0.8:
            return "partial_recovery", correction
        if correction <= 1.2:
            return "near_recovery", correction
        return "overshoot", correction

    if abs(gsf) <= 1:
        return "positive_lcio_corrected", correction
    if abs(gsf) > abs(lcio) + 0.5:
        return "positive_lcio_degraded", correction
    return "positive_lcio_other", correction


def load(args: argparse.Namespace) -> list[dict[str, object]]:
    with args.topology.open(newline="") as stream:
        topology = {
            (int(row["seed"]), int(row["entry"])): row
            for row in csv.DictReader(stream)
        }
    rows: list[dict[str, object]] = []
    with args.events.open(newline="") as stream:
        for source in csv.DictReader(stream):
            event_id = (int(source["seed"]), int(source["entry"]))
            topo = topology[event_id]
            loss = 100 * float(source["cumulative_frac"])
            lcio = float(source["lcio_residual_pct"])
            gsf = float(source["gsf_residual_pct"])
            outcome, correction = classify(source["category"], loss, lcio, gsf)
            rows.append({
                "seed": event_id[0], "entry": event_id[1],
                "category": source["category"],
                "owned_loss_pct": loss,
                "max_owned_loss_pct": 100 * float(source["max_frac"]),
                "lcio_residual_pct": lcio, "gsf_residual_pct": gsf,
                "abs_lcio_residual_pct": abs(lcio),
                "abs_gsf_residual_pct": abs(gsf),
                "absolute_error_improvement_pct": abs(lcio) - abs(gsf),
                "correction_fraction": correction,
                "outcome": outcome,
                "owned_loss_bin_pct": interval_label(loss, LOSS_BINS),
                "gsf_abs_residual_bin_pct": interval_label(abs(gsf), RESIDUAL_BINS),
                "secondary_tracker_hits": int(topo["secondary_tracker_hits"]),
                "excluded_secondary_topology": int(topo["has_secondary_tracker_activity"]),
                "ordinary_optimization_event": int(
                    not int(topo["has_secondary_tracker_activity"])
                    and abs(gsf) < 10 and abs(lcio) < 10),
            })
    return rows


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def representative_score(row, medians) -> float:
    loss_scale = max(1.0, medians[2])
    return (abs(row["owned_loss_pct"] - medians[0]) / loss_scale
            + abs(row["gsf_residual_pct"] - medians[1]) / 3.0)


def select_representatives(rows: list[dict[str, object]]) -> list[dict[str, object]]:
    cells = defaultdict(list)
    for row in rows:
        if row["category"] != "light_ebrem" or not row["ordinary_optimization_event"]:
            continue
        key = (row["owned_loss_bin_pct"], row["outcome"],
               row["gsf_abs_residual_bin_pct"])
        cells[key].append(row)
    selected = []
    for key, candidates in cells.items():
        losses = np.asarray([row["owned_loss_pct"] for row in candidates])
        residuals = np.asarray([row["gsf_residual_pct"] for row in candidates])
        medians = (float(np.median(losses)), float(np.median(residuals)),
                   float(np.quantile(losses, 0.84) - np.quantile(losses, 0.16)))
        ranked = sorted(candidates, key=lambda row: (representative_score(row, medians),
                                                      row["seed"], row["entry"]))
        for rank, row in enumerate(ranked[:3], 1):
            selected.append({
                "owned_loss_bin_pct": key[0], "outcome": key[1],
                "gsf_abs_residual_bin_pct": key[2], "cell_count": len(candidates),
                "representative_rank": rank, **row,
            })
    return selected


def summary_text(rows: list[dict[str, object]]) -> str:
    lines = [
        "Topology-clean GSF outcome survey",
        "",
        "Outcome definitions are deterministic and implemented in "
        "survey_topology_clean_gsf_outcomes.py.",
        "ordinary_optimization_event requires no secondary tracker SimHits and "
        "both |LCIO residual| and |GSF residual| below 10%.",
        "",
    ]
    for category in ("no_ebrem", "light_ebrem", "hard_ebrem"):
        subset = [row for row in rows if row["category"] == category
                  and not row["excluded_secondary_topology"]]
        ordinary = [row for row in subset if row["ordinary_optimization_event"]]
        lines.append(f"[{category}] topology_clean={len(subset)} ordinary={len(ordinary)}")
        counts = Counter(row["outcome"] for row in subset)
        lines.extend(f"  {key}: {value}" for key, value in sorted(counts.items()))
        lines.append("")
    lines.append("[light ordinary cells: owned_loss_bin / outcome / |GSF residual| bin]")
    cells = Counter((row["owned_loss_bin_pct"], row["outcome"],
                     row["gsf_abs_residual_bin_pct"])
                    for row in rows if row["category"] == "light_ebrem"
                    and row["ordinary_optimization_event"])
    lines.extend(f"  {' / '.join(key)}: {value}" for key, value in sorted(cells.items()))
    return "\n".join(lines) + "\n"


def main() -> None:
    args = arguments()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    rows = load(args)
    write_csv(args.output_dir / "topology_clean_event_outcomes.csv", rows)
    representatives = select_representatives(rows)
    write_csv(args.output_dir / "light_representative_candidates.csv", representatives)
    (args.output_dir / "survey_summary.txt").write_text(summary_text(rows))
    print(args.output_dir / "survey_summary.txt")
    print(args.output_dir / "light_representative_candidates.csv")


if __name__ == "__main__":
    main()
