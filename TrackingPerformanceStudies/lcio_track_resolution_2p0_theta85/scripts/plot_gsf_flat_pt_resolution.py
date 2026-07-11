#!/usr/bin/env python3
"""Overlay LCIO and GSF pT residuals from RecGsfFlatTuple files."""

from __future__ import annotations

import argparse
import glob
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import ROOT


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input-glob", default="gsf_flat-e--2.0-85-*.root",
        help="glob for input flat tuples (default: %(default)s)")
    parser.add_argument(
        "--output-dir",
        default="TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots",
        type=Path)
    return parser.parse_args()


def natural_seed(path: str) -> int:
    return int(Path(path).stem.rsplit("-", 1)[-1])


def main() -> None:
    args = parse_args()
    paths = sorted(glob.glob(args.input_glob), key=natural_seed)
    if not paths:
        raise SystemExit(f"No files match {args.input_glob!r}")

    chain = ROOT.TChain("gsf_tuple")
    for path in paths:
        chain.Add(path)
    arrays = ROOT.RDataFrame(chain).AsNumpy(["mc_pT", "lcio_pT", "gsf_pT"])
    truth = arrays["mc_pT"]
    lcio = arrays["lcio_pT"]
    gsf = arrays["gsf_pT"]
    valid = (np.isfinite(truth) & np.isfinite(lcio) & np.isfinite(gsf)
             & (truth > 0.0) & (lcio > 0.0) & (gsf > 0.0))
    truth, lcio, gsf = truth[valid], lcio[valid], gsf[valid]

    residuals = {
        "CompleteTracks (LCIO)": 100.0 * (lcio - truth) / truth,
        "GSF reverse mixture": 100.0 * (gsf - truth) / truth,
    }
    combined = np.concatenate(list(residuals.values()))
    low, high = np.quantile(combined, [0.001, 0.999])
    padding = 0.05 * (high - low)
    bins = np.linspace(low - padding, high + padding, 121)

    fig, ax = plt.subplots(figsize=(8.0, 6.0))
    colors = ["#276FBF", "#D1495B"]
    lines = ["--", "-"]
    summary_lines = ["sample,count,mean_pct,std_pct,median_pct,q16_pct,q84_pct,rms_pct"]
    for (label, values), color, linestyle in zip(
            residuals.items(), colors, lines):
        mean = float(np.mean(values))
        std = float(np.std(values))
        median, q16, q84 = np.quantile(values, [0.5, 0.16, 0.84])
        rms = float(np.sqrt(np.mean(values * values)))
        legend = f"{label}  median={median:.3g}%,  68%=[{q16:.3g}, {q84:.3g}]%"
        ax.hist(values, bins=bins, histtype="step", linewidth=2.0,
                color=color, linestyle=linestyle, label=legend)
        summary_lines.append(
            f"{label},{len(values)},{mean:.9g},{std:.9g},{median:.9g},"
            f"{q16:.9g},{q84:.9g},{rms:.9g}")

    ax.axvline(0.0, color="black", linewidth=1.0, alpha=0.6)
    ax.set_xlabel(r"$(p_{T}^{\mathrm{reco}}-p_{T}^{\mathrm{truth}}) / "
                  r"p_{T}^{\mathrm{truth}}$ [%]")
    ax.set_ylabel("Events / bin")
    ax.set_title(r"2 GeV $p_T$ electrons, $\theta=85^\circ$")
    ax.legend(frameon=False, fontsize=9)
    ax.grid(axis="y", alpha=0.2)
    ax.text(0.98, 0.96, f"{len(paths)} files, {len(truth)} valid events",
            transform=ax.transAxes, ha="right", va="top", fontsize=9)
    fig.tight_layout()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = args.output_dir / "gsf_reverse_vs_lcio_pt_rel_pct_resolution"
    fig.savefig(stem.with_suffix(".png"), dpi=180)
    fig.savefig(stem.with_suffix(".pdf"))

    zoom_fig, zoom_ax = plt.subplots(figsize=(8.0, 6.0))
    zoom_bins = np.linspace(-5.0, 5.0, 101)
    for (label, values), color, linestyle in zip(
            residuals.items(), colors, lines):
        inside = np.count_nonzero((values >= -5.0) & (values <= 5.0))
        zoom_ax.hist(values, bins=zoom_bins, histtype="step", linewidth=2.0,
                     color=color, linestyle=linestyle,
                     label=f"{label}  ({inside}/{len(values)} in window)")
    zoom_ax.axvline(0.0, color="black", linewidth=1.0, alpha=0.6)
    zoom_ax.set_xlim(-5.0, 5.0)
    zoom_ax.set_xlabel(r"$(p_{T}^{\mathrm{reco}}-p_{T}^{\mathrm{truth}}) / "
                       r"p_{T}^{\mathrm{truth}}$ [%]")
    zoom_ax.set_ylabel("Events / 0.1% bin")
    zoom_ax.set_title(r"2 GeV $p_T$ electrons, $\theta=85^\circ$ (core zoom)")
    zoom_ax.legend(frameon=False, fontsize=9)
    zoom_ax.grid(axis="y", alpha=0.2)
    zoom_fig.tight_layout()
    zoom_stem = stem.with_name(stem.name + "_zoom_m5_5")
    zoom_fig.savefig(zoom_stem.with_suffix(".png"), dpi=180)
    zoom_fig.savefig(zoom_stem.with_suffix(".pdf"))

    stem.with_name(stem.name + "_summary.csv").write_text(
        "\n".join(summary_lines) + "\n")
    print(f"Read {len(paths)} files; retained {len(truth)} valid events")
    print(stem.with_suffix(".png"))


if __name__ == "__main__":
    main()
