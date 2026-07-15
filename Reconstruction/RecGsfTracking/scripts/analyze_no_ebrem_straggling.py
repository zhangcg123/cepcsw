#!/usr/bin/env python3
"""Measure non-eBrem transition loss and central straggling versus t/X0.

The conditioned BH artifact deliberately assigns no-eBrem transitions to an
exact z=1 atom because deterministic mean energy loss is already enabled in
MarlinTrk.  This diagnostic subtracts the conditional median of y=-log(z) in
each t/X0 bin and measures the remaining central width.  It does not propose
that the total raw RMS, which contains rare non-eBrem loss tails, be inserted
as Gaussian process noise.
"""

import argparse
import csv
import glob
import json
import os

import matplotlib.pyplot as plt
import numpy as np


DEFAULT_EDGES = np.array(
    [0.0, 1e-4, 5e-4, 2e-3, 5e-3, 1e-2, 1.5e-2, 2e-2, 3e-2])
IDENTITY_SIGMA_Z = 1e-6


def inputs(patterns):
    paths = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if not matches:
            raise FileNotFoundError("no input matches: %s" % pattern)
        paths.extend(matches)
    return list(dict.fromkeys(os.path.abspath(path) for path in paths))


def load_no_ebrem(paths, edges):
    values = [[] for _ in range(len(edges) - 1)]
    audit = {"input_files": len(paths), "input_rows": 0, "no_ebrem_rows": 0,
             "accepted_rows": 0, "invalid_rows": 0,
             "outside_tx0_range": 0}
    for path in paths:
        with open(path, newline="") as stream:
            reader = csv.DictReader(stream)
            for row in reader:
                audit["input_rows"] += 1
                try:
                    if int(row["n_ebrem_steps"]) != 0:
                        continue
                    audit["no_ebrem_rows"] += 1
                    tx0 = float(row["g4_t_over_x0"])
                    y = float(row["minus_log_z"])
                except (KeyError, TypeError, ValueError):
                    audit["invalid_rows"] += 1
                    continue
                if not (np.isfinite(tx0) and np.isfinite(y) and tx0 >= 0):
                    audit["invalid_rows"] += 1
                    continue
                index = int(np.searchsorted(edges, tx0, side="right") - 1)
                if not 0 <= index < len(values):
                    audit["outside_tx0_range"] += 1
                    continue
                values[index].append(y)
                audit["accepted_rows"] += 1
    return [np.asarray(item) for item in values], audit


def summarize(values, edges):
    rows = []
    for index, sample in enumerate(values):
        if sample.size == 0:
            continue
        q001, q01, q16, median, q84, q99, q999 = np.quantile(
            sample, [0.001, 0.01, 0.16, 0.5, 0.84, 0.99, 0.999])
        residual = sample - median
        central68 = 0.5 * (q84 - q16)
        mad_sigma = 1.4826 * np.median(np.abs(residual))
        core = residual[(sample >= q01) & (sample <= q99)]
        rows.append({
            "tx0_low": edges[index], "tx0_high": edges[index + 1],
            "tx0_center": (np.sqrt(edges[index] * edges[index + 1])
                            if edges[index] > 0 else edges[index + 1] / 2),
            "count": sample.size, "mean_minus_log_z": np.mean(sample),
            "median_minus_log_z": median, "raw_sigma_minus_log_z": np.std(sample),
            "central68_sigma_minus_log_z": central68,
            "mad_sigma_minus_log_z": mad_sigma,
            "trimmed_1_99_sigma_minus_log_z": np.std(core),
            "q001_minus_log_z": q001, "q01_minus_log_z": q01,
            "q16_minus_log_z": q16, "q84_minus_log_z": q84,
            "q99_minus_log_z": q99, "q999_minus_log_z": q999,
            "identity_sigma_z": IDENTITY_SIGMA_Z,
            "central68_to_identity_sigma": central68 / IDENTITY_SIGMA_Z,
        })
    return rows


def plot(rows, output):
    x = np.asarray([row["tx0_center"] for row in rows])
    median = np.asarray([row["median_minus_log_z"] for row in rows])
    central = np.asarray([row["central68_sigma_minus_log_z"] for row in rows])
    mad = np.asarray([row["mad_sigma_minus_log_z"] for row in rows])
    trimmed = np.asarray([row["trimmed_1_99_sigma_minus_log_z"] for row in rows])
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.4))
    axes[0].plot(x, median, "o-", label="conditional median")
    axes[0].set_ylabel(r"median $-\log z$ (deterministic location)")
    axes[0].set_xlabel(r"transition $t/X_0$")
    axes[0].grid(alpha=0.25)
    axes[1].plot(x, central, "o-", label="central-68 half-width")
    axes[1].plot(x, mad, "s--", label="MAD-equivalent sigma")
    axes[1].plot(x, trimmed, "^:", label="1--99% trimmed RMS")
    axes[1].axhline(IDENTITY_SIGMA_Z, color="black", linestyle="--",
                    label=r"current identity $\sigma_z=10^{-6}$")
    axes[1].set_yscale("log")
    axes[1].set_ylabel(r"residual width in $-\log z$")
    axes[1].set_xlabel(r"transition $t/X_0$")
    axes[1].grid(alpha=0.25)
    axes[1].legend(fontsize=8)
    fig.suptitle("No-eBrem transition location and straggling")
    fig.tight_layout()
    fig.savefig(output, dpi=180)
    if output.lower().endswith(".png"):
        fig.savefig(output[:-4] + ".pdf")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+")
    parser.add_argument("--output-csv", required=True)
    parser.add_argument("--output-plot", required=True)
    parser.add_argument("--audit")
    args = parser.parse_args()
    paths = inputs(args.inputs)
    values, audit = load_no_ebrem(paths, DEFAULT_EDGES)
    rows = summarize(values, DEFAULT_EDGES)
    os.makedirs(os.path.dirname(os.path.abspath(args.output_csv)), exist_ok=True)
    with open(args.output_csv, "w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    audit_path = args.audit or args.output_csv + ".audit.json"
    with open(audit_path, "w") as stream:
        json.dump(audit, stream, indent=2, sort_keys=True)
        stream.write("\n")
    plot(rows, args.output_plot)
    print("wrote %d bins from %d accepted no-eBrem transitions" %
          (len(rows), audit["accepted_rows"]))


if __name__ == "__main__":
    main()
