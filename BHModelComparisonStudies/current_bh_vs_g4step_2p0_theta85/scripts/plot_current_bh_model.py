#!/usr/bin/env python3
import argparse
import csv
import math
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

HIGH_DATA = np.asarray([
    [[0.116785, 0.00300851, -0.00500615, 0.0162373, -0.0147852, 0.00507151],
     [0.929508, 0.0968065, -0.6902, 0.924948, -0.511764, 0.106812],
     [0.000895549, 0.00345136, -0.0319301, 0.0696704, -0.0659913, 0.0236099]],
    [[0.389022, -2.44128, 5.36186, -5.71814, 2.93331, -0.585089],
     [0.32526, 2.93045, -9.33945, 12.6618, -7.81376, 1.82936],
     [0.000439302, 0.0173778, 0.113849, -0.584371, 0.758494, -0.327375]],
    [[-0.0135153, 6.61798, -17.0068, 20.0236, -11.1439, 2.38296],
     [-0.554228, 8.67827, -23.9133, 30.2601, -17.6788, 3.95462],
     [-0.00209441, -0.0577078, -0.55689, 1.76433, -1.78493, 0.649452]],
    [[0.0614916, 4.70061, -12.7008, 15.3499, -8.68459, 1.87726],
     [-0.651238, 9.61423, -25.5828, 31.5933, -18.1433, 4.0],
     [6.4614e-5, 0.0279685, -0.168134, 0.52802, -0.522842, 0.192059]],
    [[0.0356266, 5.44894, -14.28, 16.9637, -9.46193, 2.02694],
     [-0.701619, 10.1539, -26.5785, 32.3834, -18.3943, 4.02389],
     [0.0044289, -0.0797969, 0.188953, -0.16965, 0.049072, 0.0024294]],
    [[-0.496513, 19.0152, -79.0763, 147.496, -128.503, 42.6193],
     [-1.1283, 14.6583, -35.5871, 40.9352, -22.1949, 4.67131],
     [0.00387964, -0.00746174, 0.0129534, -0.00999381, -0.00134676, 0.00495766]],
], dtype=float)


def poly(x, coeff):
    total = 0.0
    for c in reversed(coeff):
        total = x * total + c
    return total


def current_bh_mixture(x):
    if x < 1.0e-4:
        return [(1.0, 1.0, 0.0, "negligible")]
    if x < 0.1:
        expected_mean = math.exp(-x)
        tail_weight = min(0.20, max(0.02, 10.0 * x))
        tail_mean = (expected_mean - (1.0 - tail_weight)) / tail_weight
        tail_mean = min(0.999, max(0.50, tail_mean))
        return [
            (1.0 - tail_weight, 1.0, 0.0, "thin_no_loss"),
            (tail_weight, tail_mean, x * x, "thin_tail"),
        ]
    xx = min(x, 0.2)
    comps = []
    weight_sum = 0.0
    for i in range(6):
        w = poly(xx, HIGH_DATA[i, 0])
        m = poly(xx, HIGH_DATA[i, 1])
        v = poly(xx, HIGH_DATA[i, 2])
        comps.append([w, m, max(v, 0.0), "high_x"])
        weight_sum += w
    if weight_sum:
        for comp in comps:
            comp[0] /= weight_sum
    return [tuple(comp) for comp in comps]


def gaussian_pdf(z, mean, var, display_sigma):
    sigma = math.sqrt(var) if var > 0 else display_sigma
    sigma = max(sigma, display_sigma)
    return np.exp(-0.5 * ((z - mean) / sigma) ** 2) / (sigma * math.sqrt(2.0 * math.pi))


def weighted_quantiles(values, weights, qs):
    order = np.argsort(values)
    v = np.asarray(values)[order]
    w = np.asarray(weights)[order]
    cdf = np.cumsum(w) / np.sum(w)
    return np.interp(qs, cdf, v)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--outdir", default="BHModelComparisonStudies/current_bh_vs_g4step_2p0_theta85")
    parser.add_argument("--tx0", nargs="*", type=float, default=[5e-5, 1e-4, 3e-4, 1e-3, 3e-3, 1e-2, 3e-2, 0.1, 0.2])
    parser.add_argument("--delta-sigma", type=float, default=0.001, help="display width for zero-variance delta components")
    args = parser.parse_args()

    outdir = Path(args.outdir)
    plotdir = outdir / "plots"
    plotdir.mkdir(parents=True, exist_ok=True)

    z_grid = np.linspace(0.0, 1.05, 2000)
    rows = []
    summary_rows = []

    fig, ax = plt.subplots(figsize=(9.0, 6.2))
    colors = plt.cm.viridis(np.linspace(0.05, 0.95, len(args.tx0)))
    for color, x in zip(colors, args.tx0):
        comps = current_bh_mixture(x)
        pdf = np.zeros_like(z_grid)
        means = []
        weights = []
        for icomp, (weight, mean, var, regime) in enumerate(comps):
            pdf += weight * gaussian_pdf(z_grid, mean, var, args.delta_sigma)
            means.append(mean)
            weights.append(weight)
            rows.append({
                "tX0": x,
                "component": icomp,
                "regime": regime,
                "weight": weight,
                "mean_z": mean,
                "sigma_z": math.sqrt(var) if var > 0 else 0.0,
                "var_z": var,
            })
        q10, q50, q90 = weighted_quantiles(np.asarray(means), np.asarray(weights), [0.10, 0.50, 0.90])
        summary_rows.append({
            "tX0": x,
            "n_components": len(comps),
            "weighted_mean_z": float(np.average(means, weights=weights)),
            "component_q10_z": float(q10),
            "component_q50_z": float(q50),
            "component_q90_z": float(q90),
            "total_weight": float(np.sum(weights)),
        })
        ax.plot(z_grid, pdf, color=color, linewidth=1.8, label=f"tX0={x:g}")

    ax.set_xlabel("z = retained momentum fraction")
    ax.set_ylabel("weighted Gaussian density")
    ax.set_xlim(0.0, 1.02)
    ax.set_ylim(bottom=0.0)
    ax.grid(True, alpha=0.25)
    ax.legend(frameon=False, fontsize=8, ncol=2)
    fig.tight_layout()
    fig.savefig(plotdir / "current_bh_weighted_gaussian_curves.png", dpi=170)
    fig.savefig(plotdir / "current_bh_weighted_gaussian_curves.pdf")
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(8.2, 5.8))
    xs = np.asarray([r["tX0"] for r in summary_rows])
    mean = np.asarray([r["weighted_mean_z"] for r in summary_rows])
    q10 = np.asarray([r["component_q10_z"] for r in summary_rows])
    q50 = np.asarray([r["component_q50_z"] for r in summary_rows])
    q90 = np.asarray([r["component_q90_z"] for r in summary_rows])
    ax.plot(xs, mean, marker="o", label="weighted mean")
    ax.plot(xs, q50, marker="s", label="component median")
    ax.fill_between(xs, q10, q90, alpha=0.20, label="component q10-q90")
    ax.set_xscale("log")
    ax.set_xlabel("tX0")
    ax.set_ylabel("z")
    ax.set_ylim(0.0, 1.05)
    ax.grid(True, alpha=0.25, which="both")
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(plotdir / "current_bh_component_moments_vs_tX0.png", dpi=170)
    fig.savefig(plotdir / "current_bh_component_moments_vs_tX0.pdf")
    plt.close(fig)

    for path, data in [(outdir / "current_bh_components.csv", rows), (outdir / "current_bh_summary.csv", summary_rows)]:
        with open(path, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=list(data[0].keys()))
            writer.writeheader()
            writer.writerows(data)

    with open(outdir / "current_bh_model_summary.txt", "w") as f:
        f.write("Current BetheHeitlerSplitter.cpp model visualization\n")
        f.write("For 0.0001 <= tX0 < 0.1, current code uses a two-component CEPC thin-material toy mixture.\n")
        f.write("Zero-variance no-loss branches are drawn with a narrow display sigma only for visualization.\n\n")
        for row in summary_rows:
            f.write("tX0 %.6g n %d mean_z %.8g q10 %.8g q50 %.8g q90 %.8g\n" % (
                row["tX0"], row["n_components"], row["weighted_mean_z"],
                row["component_q10_z"], row["component_q50_z"], row["component_q90_z"]
            ))
    print(f"wrote {outdir}")


if __name__ == "__main__":
    main()
