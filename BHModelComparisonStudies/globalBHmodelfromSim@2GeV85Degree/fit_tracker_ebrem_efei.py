#!/usr/bin/env python3
import csv
import math
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import least_squares
from scipy.special import betaln, expit, logsumexp, ndtr

OUTDIR = Path("BHModelComparisonStudies/globalBHmodelfromSim@2GeV85Degree")
PLOTDIR = OUTDIR / "plots"
PLOTDIR.mkdir(parents=True, exist_ok=True)
VALUES_CSV = OUTDIR / "tracker_ebrem_efei_values.csv"
NBINS = 120
BIN_RANGE = (0.0, 1.0)
EPS = 1.0e-6

def read_values():
    values = []
    with open(VALUES_CSV) as f:
        reader = csv.DictReader(f)
        for row in reader:
            z = float(row["Ef_over_Ei"])
            if math.isfinite(z):
                values.append(min(1.0 - EPS, max(EPS, z)))
    return np.asarray(values, dtype=float)

def beta_pdf(x, a, b):
    x = np.clip(x, EPS, 1.0 - EPS)
    return np.exp((a - 1.0) * np.log(x) + (b - 1.0) * np.log1p(-x) - betaln(a, b))

def truncated_normal_pdf(x, mean, sigma, low=0.0, high=1.0):
    sigma = np.maximum(sigma, 1.0e-4)
    norm = ndtr((high - mean) / sigma) - ndtr((low - mean) / sigma)
    norm = np.maximum(norm, 1.0e-12)
    pdf = np.exp(-0.5 * ((x - mean) / sigma) ** 2) / (sigma * math.sqrt(2.0 * math.pi))
    return pdf / norm

def unpack_beta(theta, k):
    weights = np.exp(theta[:k] - logsumexp(theta[:k]))
    # Keep parameters in a broad but stable range. b below 1 allows the endpoint spike.
    alpha = np.exp(theta[k:2*k])
    beta = np.exp(theta[2*k:3*k])
    return weights, alpha, beta

def beta_mixture_bin_mass(x, width, theta, k):
    weights, alpha, beta = unpack_beta(theta, k)
    y = np.zeros_like(x)
    for w, a, b in zip(weights, alpha, beta):
        y += w * beta_pdf(x, a, b) * width
    return y

def fit_beta_mixture(x, y, width, k=3):
    # Initial components: hard-loss tail, shoulder near 0.95, endpoint peak.
    init_weights = np.array([0.16, 0.18, 0.66])
    init_alpha = np.array([3.0, 45.0, 260.0])
    init_beta = np.array([2.5, 2.6, 0.40])
    theta0 = np.r_[np.log(init_weights), np.log(init_alpha), np.log(init_beta)]

    def residual(theta):
        pred = beta_mixture_bin_mass(x, width, theta, k)
        # Weighted residual keeps the high-stat endpoint from completely hiding the tail.
        scale = np.sqrt(np.maximum(y, 1.0e-4))
        return (pred - y) / scale

    lower = np.r_[np.full(k, -20.0), np.log(np.full(k, 0.05)), np.log(np.full(k, 0.03))]
    upper = np.r_[np.full(k, 20.0), np.log(np.full(k, 800.0)), np.log(np.full(k, 50.0))]
    return least_squares(residual, theta0, bounds=(lower, upper), max_nfev=20000, xtol=1e-12, ftol=1e-12, gtol=1e-12)

def unpack_gauss(theta, k):
    weights = np.exp(theta[:k] - logsumexp(theta[:k]))
    means = expit(theta[k:2*k])
    sigmas = np.exp(theta[2*k:3*k])
    return weights, means, sigmas

def gauss_mixture_bin_mass(x, width, theta, k):
    weights, means, sigmas = unpack_gauss(theta, k)
    y = np.zeros_like(x)
    comps = []
    for w, m, s in zip(weights, means, sigmas):
        comp = w * truncated_normal_pdf(x, m, s) * width
        y += comp
        comps.append(comp)
    return y, comps

def fit_gauss_mixture_to_function(x, y_target, width, k=5):
    init_weights = np.array([0.08, 0.08, 0.10, 0.24, 0.50])
    init_means = np.array([0.45, 0.72, 0.88, 0.965, 0.997])
    init_sigmas = np.array([0.18, 0.10, 0.055, 0.018, 0.004])
    theta0 = np.r_[np.log(init_weights), np.log(init_means / (1.0 - init_means)), np.log(init_sigmas)]

    def residual(theta):
        pred, _ = gauss_mixture_bin_mass(x, width, theta, k)
        scale = np.sqrt(np.maximum(y_target, 1.0e-5))
        return (pred - y_target) / scale

    lower = np.r_[np.full(k, -25.0), np.full(k, -8.0), np.log(np.full(k, 0.0015))]
    upper = np.r_[np.full(k, 25.0), np.full(k, 12.0), np.log(np.full(k, 0.35))]
    return least_squares(residual, theta0, bounds=(lower, upper), max_nfev=30000, xtol=1e-12, ftol=1e-12, gtol=1e-12)

def chi2_like(y, pred):
    return float(np.sum(((pred - y) ** 2) / np.maximum(y, 1.0e-4)))

def main():
    values = read_values()
    counts, edges = np.histogram(values, bins=NBINS, range=BIN_RANGE)
    hist = counts / counts.sum()
    centers = 0.5 * (edges[:-1] + edges[1:])
    width = edges[1] - edges[0]

    beta_fit = fit_beta_mixture(centers, hist, width, k=3)
    beta_curve = beta_mixture_bin_mass(centers, width, beta_fit.x, 3)
    beta_w, beta_a, beta_b = unpack_beta(beta_fit.x, 3)

    gauss_fit = fit_gauss_mixture_to_function(centers, beta_curve, width, k=5)
    gauss_curve, gauss_components = gauss_mixture_bin_mass(centers, width, gauss_fit.x, 5)
    gauss_w, gauss_m, gauss_s = unpack_gauss(gauss_fit.x, 5)

    order = np.argsort(gauss_m)
    gauss_w, gauss_m, gauss_s = gauss_w[order], gauss_m[order], gauss_s[order]
    gauss_components = [gauss_components[i] for i in order]

    with open(OUTDIR / "tracker_ebrem_efei_fit_summary.txt", "w") as f:
        f.write("Fit of normalized tracker eBrem E_f/E_i histogram, electron 2 GeV theta=85 deg\n")
        f.write("Histogram is normalized by bin sum, so curves are plotted as probability per bin.\n")
        f.write("Primary smooth function: 3-component beta mixture on 0<z<1.\n")
        f.write("Gaussian mimic: 5 truncated Gaussian components normalized on 0<=z<=1 and fitted to the beta-mixture curve.\n\n")
        f.write(f"n_values {len(values)}\n")
        f.write(f"beta_chi2_like {chi2_like(hist, beta_curve):.9g}\n")
        f.write(f"gaussian_vs_beta_chi2_like {chi2_like(beta_curve, gauss_curve):.9g}\n")
        f.write(f"gaussian_vs_hist_chi2_like {chi2_like(hist, gauss_curve):.9g}\n\n")
        f.write("beta_components index weight alpha beta mean\n")
        for i, (w, a, b) in enumerate(zip(beta_w, beta_a, beta_b)):
            f.write(f"beta {i} {w:.9g} {a:.9g} {b:.9g} {a/(a+b):.9g}\n")
        f.write("\ngaussian_components index weight mean sigma\n")
        for i, (w, m, s) in enumerate(zip(gauss_w, gauss_m, gauss_s)):
            f.write(f"gaussian {i} {w:.9g} {m:.9g} {s:.9g}\n")

    with open(OUTDIR / "tracker_ebrem_efei_gaussian_components.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["component", "weight_in_0_1", "mean", "sigma", "normalization_range"])
        for i, (w, m, s) in enumerate(zip(gauss_w, gauss_m, gauss_s)):
            writer.writerow([i, f"{w:.12g}", f"{m:.12g}", f"{s:.12g}", "[0,1]"])

    with open(OUTDIR / "tracker_ebrem_efei_beta_fit_components.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["component", "weight", "alpha", "beta", "mean"])
        for i, (w, a, b) in enumerate(zip(beta_w, beta_a, beta_b)):
            writer.writerow([i, f"{w:.12g}", f"{a:.12g}", f"{b:.12g}", f"{a/(a+b):.12g}"])

    fig, ax = plt.subplots(figsize=(10.0, 7.0))
    ax.step(centers, hist, where="mid", color="black", linewidth=1.8, label="G4 tracker eBrem histogram")
    ax.plot(centers, beta_curve, color="#d62728", linewidth=2.2, label="smooth fit: 3-beta mixture")
    ax.plot(centers, gauss_curve, color="#1f77b4", linewidth=2.0, linestyle="--", label="5 truncated-Gaussian weighted sum")
    colors = plt.cm.viridis(np.linspace(0.10, 0.88, len(gauss_components)))
    for i, (comp, color, w, m, s) in enumerate(zip(gauss_components, colors, gauss_w, gauss_m, gauss_s)):
        ax.plot(centers, comp, color=color, linewidth=1.1, alpha=0.85,
                label=f"TG{i}: w={w:.3f}, mu={m:.3f}, sig={s:.3f}")
    ax.axvline(0.9, color="gray", linestyle=":", linewidth=1.4)
    ax.set_yscale("log")
    ax.set_xlim(0.0, 1.01)
    ax.set_ylim(1e-5, max(hist.max(), beta_curve.max(), gauss_curve.max()) * 2.2)
    ax.set_xlabel(r"$E_f/E_i = p_{post}/p_{pre}$")
    ax.set_ylabel("normalized steps per bin")
    ax.grid(True, alpha=0.25, which="both")
    ax.legend(frameon=False, fontsize=8, ncol=1, loc="upper left")
    fig.tight_layout()
    fig.savefig(PLOTDIR / "tracker_ebrem_Ef_over_Ei_beta_fit_gaussian_mimic.png", dpi=170)
    fig.savefig(PLOTDIR / "tracker_ebrem_Ef_over_Ei_beta_fit_gaussian_mimic.pdf")
    plt.close(fig)

    print("wrote", OUTDIR)
    print("beta chi2-like", chi2_like(hist, beta_curve))
    print("gaussian vs beta chi2-like", chi2_like(beta_curve, gauss_curve))

if __name__ == "__main__":
    main()
