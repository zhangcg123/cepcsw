#!/usr/bin/env python3
"""Compare LCIO and new GSF flat-tuple pT resolution, skipping broken files."""

from __future__ import annotations

import argparse
import csv
import glob
import math
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT


CATEGORIES = (
    ("no_ebrem", "No owned eBrem"),
    ("light_ebrem", "Light owned eBrem (<10%)"),
    ("hard_ebrem", "Hard owned eBrem (>=10%)"),
)


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-glob", default="gsf_flat-e--2.0-85-*.root")
    parser.add_argument(
        "--categories", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/full_500_sample_2026-07-12/surface_owned_ebrem_event_categories.csv"))
    parser.add_argument(
        "--topology", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13/secondary_tracker_activity_event_ids.csv"))
    parser.add_argument("--expected-entries", type=int, default=10)
    parser.add_argument("--gsf-label", default="GSF MaxComponents=24")
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/maxcomp24_new_tuples_2026-07-14"))
    return parser.parse_args()


def seed_from_path(path: str) -> int:
    match = re.search(r"[-_](\d+)\.root$", path)
    if not match:
        raise ValueError(f"Cannot extract seed from {path}")
    return int(match.group(1))


def load_reference(path: Path) -> dict[tuple[int, int], dict[str, str]]:
    with path.open(newline="") as stream:
        return {
            (int(row["seed"]), int(row["entry"])): row
            for row in csv.DictReader(stream)
        }


def load_secondary_ids(path: Path) -> set[tuple[int, int]]:
    with path.open(newline="") as stream:
        return {
            (int(row["seed"]), int(row["entry"]))
            for row in csv.DictReader(stream)
            if int(row["has_secondary_tracker_activity"])
        }


def read_tuples(args: argparse.Namespace, reference, secondary_ids):
    paths = sorted(glob.glob(args.input_glob), key=seed_from_path)
    if not paths:
        raise RuntimeError(f"No files match {args.input_glob!r}")
    ROOT.gErrorIgnoreLevel = ROOT.kFatal
    rows, audit = [], []
    for path in paths:
        seed = seed_from_path(path)
        root_file = None
        status, entries, detail = "usable", -1, ""
        try:
            root_file = ROOT.TFile.Open(path)
        except OSError as error:
            status, detail = "open_error", str(error)
        if status == "usable" and (not root_file or root_file.IsZombie()):
            status = "zombie"
        tree = root_file.Get("gsf_tuple") if status == "usable" else None
        if status == "usable" and not tree:
            status = "missing_tree"
        if status == "usable":
            entries = int(tree.GetEntries())
            if entries != args.expected_entries:
                status = "unexpected_entry_count"
                detail = f"expected {args.expected_entries}, found {entries}"
        audit.append({
            "seed": seed, "path": path, "size_bytes": Path(path).stat().st_size,
            "entries": entries, "status": status, "detail": detail,
        })
        if status == "usable":
            for entry in range(entries):
                event_id = (seed, entry)
                if event_id not in reference:
                    raise RuntimeError(f"No reference category for {event_id}")
                tree.GetEntry(entry)
                truth = float(tree.mc_pT)
                lcio = float(tree.lcio_pT)
                gsf = float(tree.gsf_pT)
                if not all(math.isfinite(x) and x > 0 for x in (truth, lcio, gsf)):
                    continue
                source = reference[event_id]
                rows.append({
                    "seed": seed, "entry": entry,
                    "category": source["category"],
                    "excluded_secondary_topology": int(event_id in secondary_ids),
                    "mc_pT": truth, "lcio_pT": lcio, "gsf_pT": gsf,
                    "lcio_residual_pct": 100 * (lcio / truth - 1),
                    "gsf_residual_pct": 100 * (gsf / truth - 1),
                })
        if root_file:
            root_file.Close()
    return rows, audit


def stats(values: np.ndarray) -> dict[str, float | int]:
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    return {
        "count": values.size, "median_pct": median, "q16_pct": q16,
        "q84_pct": q84, "width68_pct": 0.5 * (q84 - q16),
        "rms_pct": np.sqrt(np.mean(values * values)),
        "inside_1pct": np.count_nonzero(np.abs(values) <= 1),
        "inside_2pct": np.count_nonzero(np.abs(values) <= 2),
        "inside_5pct": np.count_nonzero(np.abs(values) <= 5),
        "inside_10pct": np.count_nonzero(np.abs(values) <= 10),
    }


def plot_sample(rows, output: Path, topology_clean: bool,
                gsf_label: str) -> list[dict]:
    selected_rows = [
        row for row in rows
        if not topology_clean or not row["excluded_secondary_topology"]
    ]
    summaries = []
    for zoom in (False, True):
        fig, axes = plt.subplots(1, 3, figsize=(16.5, 5.3))
        for ax, (category, title) in zip(axes, CATEGORIES):
            category_rows = [r for r in selected_rows if r["category"] == category]
            lcio = np.asarray([r["lcio_residual_pct"] for r in category_rows])
            gsf = np.asarray([r["gsf_residual_pct"] for r in category_rows])
            if zoom:
                bins = np.linspace(-5, 5, 101)
            else:
                low, high = np.quantile(np.concatenate((lcio, gsf)), [0.001, 0.999])
                padding = max(0.25, 0.04 * (high - low))
                bins = np.linspace(low - padding, high + padding, 101)
            for algorithm, values, color, linestyle in (
                    ("LCIO", lcio, "#276FBF", "--"),
                    (gsf_label, gsf, "#D1495B", "-")):
                result = stats(values)
                ax.hist(values, bins=bins, histtype="step", linewidth=2,
                        color=color, linestyle=linestyle,
                        label=(f"{algorithm}: median {result['median_pct']:.3g}%, "
                               f"width68 {result['width68_pct']:.3g}%"))
                if not zoom:
                    summaries.append({
                        "population": "topology_clean" if topology_clean else "inclusive",
                        "category": category, "algorithm": algorithm, **result,
                    })
            ax.axvline(0, color="black", linewidth=1, alpha=0.6)
            if zoom:
                ax.set_xlim(-5, 5)
            ax.set_title(f"{title}\nN = {len(category_rows)}")
            ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
            ax.grid(axis="y", alpha=0.2)
            ax.legend(frameon=False, fontsize=8.5)
        axes[0].set_ylabel("Events / bin")
        population = "secondary tracker activity excluded" if topology_clean else "inclusive"
        suffix = " (core zoom)" if zoom else ""
        fig.suptitle(
            rf"New {gsf_label} tuples: 2 GeV $p_T$ electrons, $\theta=85^\circ$"
            f"\nSurface-owned Geant4 eBrem categories; {population}{suffix}")
        fig.tight_layout()
        path = output.with_name(output.name + ("_zoom_m5_5" if zoom else ""))
        fig.savefig(path.with_suffix(".png"), dpi=180)
        fig.savefig(path.with_suffix(".pdf"))
        plt.close(fig)
    return summaries


def plot_inclusive_all(rows, output: Path, gsf_label: str) -> list[dict]:
    """Plot one fully inclusive distribution without category splitting."""
    lcio = np.asarray([row["lcio_residual_pct"] for row in rows])
    gsf = np.asarray([row["gsf_residual_pct"] for row in rows])
    summaries = []
    for algorithm, values in (("LCIO", lcio), (gsf_label, gsf)):
        summaries.append({"population": "inclusive_all", "category": "all",
                          "algorithm": algorithm, **stats(values)})
    for zoom in (False, True):
        fig, ax = plt.subplots(figsize=(8.4, 6.2))
        if zoom:
            bins = np.linspace(-5, 5, 101)
        else:
            low, high = np.quantile(np.concatenate((lcio, gsf)), [0.001, 0.999])
            padding = max(0.25, 0.04 * (high - low))
            bins = np.linspace(low - padding, high + padding, 121)
        for algorithm, values, color, linestyle in (
                ("LCIO", lcio, "#276FBF", "--"),
                (gsf_label, gsf, "#D1495B", "-")):
            displayed = values[(values >= -5) & (values <= 5)] if zoom else values
            result = stats(displayed)
            if zoom:
                summaries.append({
                    "population": "inclusive_all_zoom_m5_5",
                    "category": "all", "algorithm": algorithm, **result,
                })
            ax.hist(values, bins=bins, histtype="step", linewidth=2,
                    color=color, linestyle=linestyle,
                    label=(f"{algorithm} (N={result['count']}): "
                           f"median {result['median_pct']:.3g}%, "
                           f"width68 {result['width68_pct']:.3g}%"))
        ax.axvline(0, color="black", linewidth=1, alpha=0.6)
        if zoom:
            ax.set_xlim(-5, 5)
        ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        ax.set_ylabel("Events / bin")
        ax.set_title(
            r"2 GeV $p_T$ electrons, $\theta=85^\circ$: all valid events"
            + (" (core zoom)" if zoom else ""))
        ax.text(0.98, 0.96, f"Input N = {len(rows)}", transform=ax.transAxes,
                ha="right", va="top")
        ax.grid(axis="y", alpha=0.2)
        ax.legend(frameon=False)
        fig.tight_layout()
        path = output.with_name(output.name + ("_zoom_m5_5" if zoom else ""))
        fig.savefig(path.with_suffix(".png"), dpi=180)
        fig.savefig(path.with_suffix(".pdf"))
        plt.close(fig)
    return summaries


def write_csv(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    args = arguments()
    reference = load_reference(args.categories)
    secondary_ids = load_secondary_ids(args.topology)
    rows, audit = read_tuples(args, reference, secondary_ids)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_csv(args.output_dir / "input_file_audit.csv", audit)
    write_csv(args.output_dir / "matched_event_residuals.csv", rows)
    summaries = []
    summaries += plot_inclusive_all(
        rows, args.output_dir / "inclusive_all_events_pt_resolution", args.gsf_label)
    summaries += plot_sample(
        rows, args.output_dir / "inclusive_pt_resolution", False, args.gsf_label)
    summaries += plot_sample(
        rows, args.output_dir / "topology_clean_pt_resolution", True, args.gsf_label)
    write_csv(args.output_dir / "pt_resolution_summary.csv", summaries)
    broken = [row for row in audit if row["status"] != "usable"]
    print(f"Input files: {len(audit)}; usable: {len(audit) - len(broken)}; broken: {len(broken)}")
    print(f"Matched valid events: {len(rows)}")
    print("Broken seeds:", ",".join(str(row["seed"]) for row in broken))
    print(args.output_dir / "pt_resolution_summary.csv")


if __name__ == "__main__":
    main()
