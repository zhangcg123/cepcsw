#!/usr/bin/env python3
"""Compare eBrem-attributed energy loss per radiation length between samples."""

import argparse
import csv
import glob
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def expand(pattern):
    paths = sorted(glob.glob(pattern))
    if not paths:
        raise FileNotFoundError(pattern)
    return paths


def read(pattern):
    transition_abs, transition_frac = [], []
    events = defaultdict(lambda: [0.0, 0.0, 0.0])
    rows = ebrem_rows = 0
    for path in expand(pattern):
        with open(path, newline="") as stream:
            for row in csv.DictReader(stream):
                tx0 = float(row["g4_t_over_x0"])
                before = float(row["p_before_GeV"])
                loss = float(row["ebrem_step_loss_sum_GeV"])
                if not (0.0 < tx0 < 0.03 and before > 0.0):
                    continue
                rows += 1
                key = (row["source_file"], row["run_id"], row["event_id"],
                       row["track_id"])
                events[key][0] += tx0
                events[key][1] += loss
                events[key][2] = max(events[key][2], before)
                if int(row["n_ebrem_steps"]) <= 0 or loss <= 0.0:
                    continue
                ebrem_rows += 1
                transition_abs.append(loss / tx0)
                transition_frac.append((loss / before) / tx0)
    event_abs, event_frac = [], []
    for tx0, loss, momentum in events.values():
        if tx0 > 0.0 and loss > 0.0 and momentum > 0.0:
            event_abs.append(loss / tx0)
            event_frac.append((loss / momentum) / tx0)
    return {
        "transition_abs": np.asarray(transition_abs),
        "transition_frac": np.asarray(transition_frac),
        "event_abs": np.asarray(event_abs), "event_frac": np.asarray(event_frac),
        "rows": rows, "ebrem_rows": ebrem_rows, "events": len(events),
        "ebrem_events": len(event_abs),
    }


def log_edges(samples):
    joined = np.concatenate([sample for sample in samples if sample.size])
    low, high = np.quantile(joined, [0.001, 0.999])
    return np.geomspace(max(low, 1e-12), high, 90)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sample", action="append", nargs=2,
                        metavar=("LABEL", "GLOB"), required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--title", default=r"Global eBrem energy-loss rate, $t/X_0<0.03$")
    args = parser.parse_args()
    data = [(label, read(pattern)) for label, pattern in args.sample]
    fields = ["transition_abs", "transition_frac", "event_abs", "event_frac"]
    titles = [
        r"eBrem transitions: $\Delta E/(t/X_0)$",
        r"eBrem transitions: $(\Delta E/p)/(t/X_0)$",
        r"events: $\sum\Delta E/\sum(t/X_0)$",
        r"events: $(\sum\Delta E/p_0)/\sum(t/X_0)$",
    ]
    units = ["GeV", "", "GeV", ""]
    fig, axes = plt.subplots(2, 2, figsize=(11, 8.3))
    for ax, field, title, unit in zip(axes.flat, fields, titles, units):
        edges = log_edges([sample[field] for _, sample in data])
        for label, sample in data:
            values = sample[field]
            weights = np.full(values.size, 1.0 / values.size)
            ax.hist(values, bins=edges, weights=weights, histtype="step",
                    linewidth=2, label=f"{label} (N={values.size})")
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_title(title)
        ax.set_xlabel("loss per radiation length" + (f" [{unit}]" if unit else ""))
        ax.set_ylabel("probability / log bin")
        ax.grid(alpha=0.2)
        ax.legend(frameon=False, fontsize=8)
    fig.suptitle(args.title)
    fig.tight_layout()
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output, dpi=180, facecolor="white")
    fig.savefig(output.with_suffix(".pdf"), facecolor="white")
    summary = output.with_name(output.stem + "_summary.csv")
    with summary.open("w") as stream:
        stream.write("sample,transitions,ebrem_transitions,events,ebrem_events,"
                     "median_transition_abs_GeV_per_X0,median_transition_frac_per_X0,"
                     "median_event_abs_GeV_per_X0,median_event_frac_per_X0\n")
        for label, sample in data:
            medians = [np.median(sample[field]) for field in fields]
            stream.write(f"{label},{sample['rows']},{sample['ebrem_rows']},"
                         f"{sample['events']},{sample['ebrem_events']}," +
                         ",".join(f"{value:.10g}" for value in medians) + "\n")
    print(output)
    print(summary)


if __name__ == "__main__":
    main()
