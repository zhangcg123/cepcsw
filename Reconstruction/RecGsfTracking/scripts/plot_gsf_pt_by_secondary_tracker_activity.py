#!/usr/bin/env python3
"""Separate GSF pT resolution by secondary tracker SimHit activity."""

from __future__ import annotations

import argparse
import csv
import glob
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT


COLLECTIONS = {
    "vxd": "_VXDCollection_MCParticle.index",
    "itk_barrel": "_ITKBarrelCollection_MCParticle.index",
    "itk_endcap": "_ITKEndcapCollection_MCParticle.index",
    "tpc": "_TPCCollection_MCParticle.index",
    "otk_barrel": "_OTKBarrelCollection_MCParticle.index",
    "otk_endcap": "_OTKEndcapCollection_MCParticle.index",
}


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sim-glob", default="tuples285/sim-e--2.0-85-*.root")
    parser.add_argument(
        "--category-csv",
        type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/full_500_sample_2026-07-12/surface_owned_ebrem_event_categories.csv"))
    parser.add_argument(
        "--output-dir", type=Path,
        default=Path("TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/secondary_tracker_activity_2026-07-13"))
    parser.add_argument("--complex-threshold", type=int, default=20)
    return parser.parse_args()


def seed_from_path(path: str) -> int:
    return int(Path(path).stem.rsplit("-", 1)[-1])


def build_topology(paths: list[str]) -> list[dict[str, int]]:
    rows: list[dict[str, int]] = []
    for path in sorted(paths, key=seed_from_path):
        source = ROOT.TFile.Open(path)
        tree = source.Get("events") if source else None
        if not tree:
            raise RuntimeError(f"Missing events tree in {path}")
        formulas = {
            name: ROOT.TTreeFormula(
                f"n_{name}", f"Sum$({branch}>0)", tree)
            for name, branch in COLLECTIONS.items()
        }
        n_mc = ROOT.TTreeFormula("n_mc", "Length$(MCParticle.PDG)", tree)
        for entry in range(tree.GetEntries()):
            tree.GetEntry(entry)
            counts = {name: int(formula.EvalInstance())
                      for name, formula in formulas.items()}
            rows.append({
                "seed": seed_from_path(path),
                "entry": entry,
                "n_mc_particles": int(n_mc.EvalInstance()),
                **{f"secondary_{name}_hits": value
                   for name, value in counts.items()},
                "secondary_tracker_hits": sum(counts.values()),
            })
        source.Close()
    return rows


def write_topology(path: Path, rows: list[dict[str, int]], threshold: int) -> None:
    fields = list(rows[0]) + ["has_secondary_tracker_activity", "complex_topology"]
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            n = row["secondary_tracker_hits"]
            writer.writerow({**row,
                             "has_secondary_tracker_activity": int(n > 0),
                             "complex_topology": int(n >= threshold)})


def summarize(values: np.ndarray) -> tuple[float, float, float, float]:
    q16, median, q84 = np.quantile(values, [0.16, 0.5, 0.84])
    rms = np.sqrt(np.mean(values * values))
    return float(median), float(q16), float(q84), float(rms)


def plot_residuals(output: Path, topology: list[dict[str, int]],
                   category_csv: Path, threshold: int) -> None:
    topology_by_id = {(row["seed"], row["entry"]): row for row in topology}
    groups: dict[str, list[float]] = {
        "No secondary tracker SimHits": [],
        "Secondary tracker activity (>0 hits)": [],
        f"Complex subset (≥{threshold} hits)": [],
    }
    unmatched: list[tuple[int, int]] = []
    with category_csv.open(newline="") as stream:
        for row in csv.DictReader(stream):
            key = (int(row["seed"]), int(row["entry"]))
            topo = topology_by_id.get(key)
            if topo is None:
                unmatched.append(key)
                continue
            residual = float(row["gsf_residual_pct"])
            if not np.isfinite(residual):
                continue
            n = topo["secondary_tracker_hits"]
            if n == 0:
                groups["No secondary tracker SimHits"].append(residual)
            else:
                groups["Secondary tracker activity (>0 hits)"].append(residual)
                if n >= threshold:
                    groups[f"Complex subset (≥{threshold} hits)"].append(residual)
    if unmatched:
        raise RuntimeError(f"Missing SimHit topology for {len(unmatched)} IDs")

    arrays = {label: np.asarray(values) for label, values in groups.items()}
    all_values = np.concatenate([values for values in arrays.values() if len(values)])
    low, high = np.quantile(all_values, [0.001, 0.999])
    padding = 0.05 * (high - low)
    bins = np.linspace(low - padding, high + padding, 121)
    styles = [("#276FBF", "-"), ("#D1495B", "-"), ("#6A4C93", "--")]

    summary = ["sample,count,median_pct,q16_pct,q84_pct,rms_pct,inside_1pct,inside_5pct"]
    fig, ax = plt.subplots(figsize=(8.4, 6.2))
    for (label, values), (color, linestyle) in zip(arrays.items(), styles):
        if not len(values):
            continue
        median, q16, q84, rms = summarize(values)
        summary.append(
            f"{label},{len(values)},{median:.9g},{q16:.9g},{q84:.9g},{rms:.9g},"
            f"{np.count_nonzero(np.abs(values) <= 1)},{np.count_nonzero(np.abs(values) <= 5)}")
        ax.hist(values, bins=bins, density=True, histtype="step", linewidth=2,
                color=color, linestyle=linestyle,
                label=f"{label} (N={len(values)}, median={median:.3g}%)")
    ax.axvline(0, color="black", linewidth=1, alpha=0.6)
    ax.set_xlabel(r"$(p_T^{\mathrm{GSF}}-p_T^{\mathrm{truth}})/p_T^{\mathrm{truth}}$ [%]")
    ax.set_ylabel("Normalized event density")
    ax.set_title(r"2 GeV $p_T$ electrons, $\theta=85^\circ$: tracker-secondary topology")
    ax.legend(frameon=False, fontsize=8.5)
    ax.grid(axis="y", alpha=0.2)
    fig.tight_layout()
    fig.savefig(output.with_suffix(".png"), dpi=180)
    fig.savefig(output.with_suffix(".pdf"))
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(8.4, 6.2))
    bins = np.linspace(-10, 10, 101)
    for (label, values), (color, linestyle) in zip(arrays.items(), styles):
        if len(values):
            ax.hist(values, bins=bins, density=True, histtype="step", linewidth=2,
                    color=color, linestyle=linestyle, label=f"{label} (N={len(values)})")
    ax.axvline(0, color="black", linewidth=1, alpha=0.6)
    ax.set_xlim(-10, 10)
    ax.set_xlabel(r"$(p_T^{\mathrm{GSF}}-p_T^{\mathrm{truth}})/p_T^{\mathrm{truth}}$ [%]")
    ax.set_ylabel("Normalized event density")
    ax.set_title(r"2 GeV $p_T$ electrons, $\theta=85^\circ$ (±10% zoom)")
    ax.legend(frameon=False, fontsize=8.5)
    ax.grid(axis="y", alpha=0.2)
    fig.tight_layout()
    zoom = output.with_name(output.name + "_zoom_m10_10")
    fig.savefig(zoom.with_suffix(".png"), dpi=180)
    fig.savefig(zoom.with_suffix(".pdf"))
    plt.close(fig)
    output.with_name(output.name + "_summary.csv").write_text("\n".join(summary) + "\n")


def main() -> None:
    args = arguments()
    paths = glob.glob(args.sim_glob)
    if not paths:
        raise SystemExit(f"No files match {args.sim_glob}")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    topology = build_topology(paths)
    topology_csv = args.output_dir / "secondary_tracker_activity_event_ids.csv"
    write_topology(topology_csv, topology, args.complex_threshold)
    output = args.output_dir / "gsf_pt_resolution_by_secondary_tracker_activity"
    plot_residuals(output, topology, args.category_csv, args.complex_threshold)
    print(f"Wrote {len(topology)} topology rows to {topology_csv}")
    print(output.with_suffix(".png"))


if __name__ == "__main__":
    main()
