#!/usr/bin/env python3
"""Make a matched-event pT comparison of MaxComponents 12 and 24."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    base = "TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots"
    parser.add_argument("--max12", type=Path, default=Path(
        base + "/maxcomp12_new_tuples_2026-07-14/matched_event_residuals.csv"))
    parser.add_argument("--max24", type=Path, default=Path(
        base + "/maxcomp24_new_tuples_2026-07-14/matched_event_residuals.csv"))
    parser.add_argument("--output-dir", type=Path, default=Path(
        base + "/maxcomp12_vs_24_matched_2026-07-14"))
    parser.add_argument("--first-label", default="GSF MaxComponents=12")
    parser.add_argument("--second-label", default="GSF MaxComponents=24")
    parser.add_argument("--title", default="Matched events: MaxComponents 12 versus 24")
    return parser.parse_args()


def read(path: Path) -> dict[tuple[int, int], dict[str, str]]:
    with path.open(newline="") as stream:
        return {(int(r["seed"]), int(r["entry"])): r for r in csv.DictReader(stream)}


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


def write(path: Path, rows: list[dict]) -> None:
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def gaussian_core_fit(values: np.ndarray, low: float = -2.0,
                      high: float = 2.0, bins: int = 80) -> dict[str, float | int]:
    """Fit a Gaussian to the inclusive core with Poisson bin uncertainties."""
    counts, edges = np.histogram(values, bins=bins, range=(low, high))
    centers = 0.5 * (edges[:-1] + edges[1:])
    widths = np.diff(edges)
    mask = counts > 0

    def model(x, norm, mean, sigma):
        return norm * np.exp(-0.5 * ((x - mean) / sigma) ** 2)

    core = values[(values >= low) & (values <= high)]
    initial = (float(counts.max()), float(np.median(core)),
               max(float(np.std(core)), 0.05))
    parameters, covariance = curve_fit(
        model, centers[mask], counts[mask], p0=initial,
        sigma=np.sqrt(counts[mask]), absolute_sigma=True,
        bounds=([0.0, low, 1e-4], [np.inf, high, high - low]),
        maxfev=20000)
    errors = np.sqrt(np.diag(covariance))
    expected = model(centers[mask], *parameters)
    chi2 = float(np.sum((counts[mask] - expected) ** 2 / counts[mask]))
    ndf = int(np.count_nonzero(mask) - len(parameters))
    return {
        "fit_low_pct": low, "fit_high_pct": high, "histogram_bins": bins,
        "entries_total": values.size, "entries_in_fit_range": core.size,
        "amplitude": parameters[0], "amplitude_error": errors[0],
        "mean_pct": parameters[1], "mean_error_pct": errors[1],
        "sigma_pct": parameters[2], "sigma_error_pct": errors[2],
        "chi2": chi2, "ndf": ndf, "chi2_ndf": chi2 / ndf,
        "bin_width_pct": float(widths[0]),
    }


def main() -> None:
    args = arguments()
    twelve, twenty_four = read(args.max12), read(args.max24)
    common = sorted(set(twelve) & set(twenty_four))
    rows = []
    for event_id in common:
        r12, r24 = twelve[event_id], twenty_four[event_id]
        rows.append({
            "seed": event_id[0], "entry": event_id[1],
            "category": r12["category"],
            "excluded_secondary_topology": r12["excluded_secondary_topology"],
            "lcio_residual_pct": float(r12["lcio_residual_pct"]),
            "first_residual_pct": float(r12["gsf_residual_pct"]),
            "second_residual_pct": float(r24["gsf_residual_pct"]),
            "abs_error_change_first_minus_second_pct": (
                abs(float(r12["gsf_residual_pct"]))
                - abs(float(r24["gsf_residual_pct"]))),
        })
    args.output_dir.mkdir(parents=True, exist_ok=True)
    write(args.output_dir / "matched_event_residuals.csv", rows)

    fields = (("LCIO", "lcio_residual_pct", "#276FBF", "--"),
              (args.first_label, "first_residual_pct", "#F28E2B", "-"),
              (args.second_label, "second_residual_pct", "#D1495B", "-"))
    inclusive_arrays = {
        label: np.asarray([r[field] for r in rows])
        for label, field, _, _ in fields
    }
    gaussian_fits = [
        {"algorithm": label, **gaussian_core_fit(values)}
        for label, values in inclusive_arrays.items()
    ]
    write(args.output_dir / "inclusive_gaussian_core_fits.csv", gaussian_fits)
    paired_summary = []
    for population, selected in (
            ("inclusive_all", rows),
            ("topology_clean_all", [r for r in rows if not int(r["excluded_secondary_topology"])]),
            ("secondary_activity", [r for r in rows if int(r["excluded_secondary_topology"])])):
        for category in ("all", "no_ebrem", "light_ebrem", "hard_ebrem"):
            subset = selected if category == "all" else [
                r for r in selected if r["category"] == category]
            if not subset:
                continue
            first = np.asarray([r["first_residual_pct"] for r in subset])
            second = np.asarray([r["second_residual_pct"] for r in subset])
            first_abs, second_abs = np.abs(first), np.abs(second)
            tolerance = 1e-12
            paired_summary.append({
                "population": population, "category": category,
                "count": len(subset),
                "first_mean_abs_residual_pct": np.mean(first_abs),
                "second_mean_abs_residual_pct": np.mean(second_abs),
                "first_wins": np.count_nonzero(first_abs + tolerance < second_abs),
                "second_wins": np.count_nonzero(second_abs + tolerance < first_abs),
                "ties": np.count_nonzero(np.abs(first_abs - second_abs) <= tolerance),
                "second_minus_first_mean_residual_pct": np.mean(second - first),
                "second_minus_first_median_residual_pct": np.median(second - first),
                "second_lower_pt": np.count_nonzero(second < first - tolerance),
                "second_higher_pt": np.count_nonzero(second > first + tolerance),
                "second_same_pt": np.count_nonzero(np.abs(second - first) <= tolerance),
                "first_beyond_10pct": np.count_nonzero(first_abs > 10),
                "second_beyond_10pct": np.count_nonzero(second_abs > 10),
                "first_beyond_50pct": np.count_nonzero(first_abs > 50),
                "second_beyond_50pct": np.count_nonzero(second_abs > 50),
                "residual_correlation": np.corrcoef(first, second)[0, 1],
            })
    write(args.output_dir / "paired_comparison_summary.csv", paired_summary)
    summary = []
    for population, selected in (
            ("inclusive_all", rows),
            ("topology_clean_all", [r for r in rows if not int(r["excluded_secondary_topology"])])):
        for category in ("all", "no_ebrem", "light_ebrem", "hard_ebrem"):
            subset = selected if category == "all" else [r for r in selected if r["category"] == category]
            for label, field, _, _ in fields:
                summary.append({"population": population, "category": category,
                                "algorithm": label,
                                **stats(np.asarray([r[field] for r in subset]))})

    for zoom in (False, True):
        fig, ax = plt.subplots(figsize=(8.5, 6.2))
        arrays = [np.asarray([r[field] for r in rows]) for _, field, _, _ in fields]
        if zoom:
            bins = np.linspace(-5, 5, 101)
        else:
            low, high = np.quantile(np.concatenate(arrays), [0.001, 0.999])
            padding = max(0.25, 0.04 * (high - low))
            bins = np.linspace(low - padding, high + padding, 121)
        for (label, _, color, linestyle), values in zip(fields, arrays):
            displayed = values[(values >= -5) & (values <= 5)] if zoom else values
            result = stats(displayed)
            if zoom:
                summary.append({"population": "inclusive_all_zoom_m5_5",
                                "category": "all", "algorithm": label, **result})
            ax.hist(values, bins=bins, histtype="step", linewidth=2,
                    color=color, linestyle=linestyle,
                    label=(f"{label} (N={result['count']}): median "
                           f"{result['median_pct']:.3g}%, w68 "
                           f"{result['width68_pct']:.3g}%"))
            if zoom:
                fit = next(r for r in gaussian_fits if r["algorithm"] == label)
                x = np.linspace(fit["fit_low_pct"], fit["fit_high_pct"], 400)
                y = fit["amplitude"] * np.exp(
                    -0.5 * ((x - fit["mean_pct"]) / fit["sigma_pct"]) ** 2)
                ax.plot(x, y, color=color, linewidth=1.2, alpha=0.8,
                        label=(f"{label} Gaussian: mu={fit['mean_pct']:.3g}%, "
                               f"sigma={fit['sigma_pct']:.3g}%"))
        ax.axvline(0, color="black", linewidth=1, alpha=0.6)
        if zoom:
            ax.set_xlim(-5, 5)
        ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
        ax.set_ylabel("Events / bin")
        ax.set_title(args.title + (" (core zoom)" if zoom else ""))
        ax.text(0.98, 0.96, f"Common input N = {len(rows)}",
                transform=ax.transAxes, ha="right", va="top")
        ax.grid(axis="y", alpha=0.2)
        ax.legend(frameon=False, fontsize=8.5)
        fig.tight_layout()
        stem = args.output_dir / "inclusive_matched_pt_resolution"
        if zoom:
            stem = stem.with_name(stem.name + "_zoom_m5_5")
        fig.savefig(stem.with_suffix(".png"), dpi=180)
        fig.savefig(stem.with_suffix(".pdf"))
        plt.close(fig)

    category_titles = {
        "no_ebrem": "No owned eBrem",
        "light_ebrem": "Light owned eBrem (<10%)",
        "hard_ebrem": "Hard owned eBrem (>=10%)",
    }
    clean_rows = [r for r in rows if not int(r["excluded_secondary_topology"])]
    for zoom in (False, True):
        fig, axes = plt.subplots(1, 3, figsize=(16.5, 5.3))
        for ax, category in zip(axes, category_titles):
            subset = [r for r in clean_rows if r["category"] == category]
            arrays = [np.asarray([r[field] for r in subset])
                      for _, field, _, _ in fields]
            if zoom:
                bins = np.linspace(-5, 5, 101)
            else:
                low, high = np.quantile(np.concatenate(arrays), [0.001, 0.999])
                padding = max(0.25, 0.04 * (high - low))
                bins = np.linspace(low - padding, high + padding, 101)
            for (label, _, color, linestyle), values in zip(fields, arrays):
                result = stats(values)
                ax.hist(values, bins=bins, histtype="step", linewidth=2,
                        color=color, linestyle=linestyle,
                        label=(f"{label}: median {result['median_pct']:.3g}%, "
                               f"w68 {result['width68_pct']:.3g}%"))
            ax.axvline(0, color="black", linewidth=1, alpha=0.6)
            if zoom:
                ax.set_xlim(-5, 5)
            ax.set_title(f"{category_titles[category]}\nN = {len(subset)}")
            ax.set_xlabel(r"$(p_T^{reco}-p_T^{truth})/p_T^{truth}$ [%]")
            ax.grid(axis="y", alpha=0.2)
            ax.legend(frameon=False, fontsize=8)
        axes[0].set_ylabel("Events / bin")
        fig.suptitle(args.title + "; secondary tracker activity excluded"
                     + (" (core zoom)" if zoom else ""))
        fig.tight_layout()
        stem = args.output_dir / "topology_clean_category_pt_resolution"
        if zoom:
            stem = stem.with_name(stem.name + "_zoom_m5_5")
        fig.savefig(stem.with_suffix(".png"), dpi=180)
        fig.savefig(stem.with_suffix(".pdf"))
        plt.close(fig)
    write(args.output_dir / "matched_pt_resolution_summary.csv", summary)
    print(f"{args.first_label} events: {len(twelve)}; "
          f"{args.second_label} events: {len(twenty_four)}")
    print(f"Common matched events: {len(rows)}")
    print(args.output_dir / "matched_pt_resolution_summary.csv")


if __name__ == "__main__":
    main()
