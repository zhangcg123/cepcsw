#!/usr/bin/env python3
"""Analyze GsfMaterialStepRecorderAnaElemTool g4step_tuple files.

The tuple stores true Geant4 pre/post-step momentum information, one TTree
entry per event and vector branches for the material steps. This script is the
main diagnostic path for CEPC Bethe-Heitler tuning studies; SimTrackerHit
momentum summaries should only be used as detector-level cross-checks.
"""
from __future__ import annotations

import argparse
import csv
import math
import os
from collections import Counter
from dataclasses import dataclass, field
from typing import Iterable, List, Sequence, Tuple

import ROOT

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

EPS = 1.0e-12
TAIL_THRESHOLDS = (0.999, 0.99, 0.95, 0.90)
LOSS_THRESHOLDS_GEV = (1.0e-6, 1.0e-3, 1.0e-2, 5.0e-2, 1.0e-1)


@dataclass
class StepRecord:
    event_id: int
    index: int
    track_id: int
    parent_id: int
    pdg: int
    pre_p: float
    post_p: float
    pre_r: float
    post_r: float
    post_z: float
    z: float
    loss: float
    step_tx0: float
    process: str
    material: str


@dataclass
class EventRecord:
    event_id: int
    n_all: int
    n_primary: int
    loss_all: float
    loss_primary: float
    loss_ebrem_primary: float
    tx0_all: float
    tx0_primary: float
    endpoint_retained_primary: float
    endpoint_r_primary: float
    endpoint_z_primary: float
    first_primary_p: float
    last_primary_p: float


@dataclass
class SampleSummary:
    label: str
    filename: str
    entries: int = 0
    steps: List[StepRecord] = field(default_factory=list)
    events: List[EventRecord] = field(default_factory=list)
    process_counts: Counter = field(default_factory=Counter)
    material_counts: Counter = field(default_factory=Counter)

    def all_z(self) -> List[float]:
        return [s.z for s in self.steps if math.isfinite(s.z)]

    def all_loss(self) -> List[float]:
        return [s.loss for s in self.steps if math.isfinite(s.loss)]

    def all_tx0(self) -> List[float]:
        return [s.step_tx0 for s in self.steps if math.isfinite(s.step_tx0)]

    def primary_steps(self) -> List[StepRecord]:
        return [s for s in self.steps if is_primary_step(s)]

    def primary_z(self) -> List[float]:
        return [s.z for s in self.primary_steps() if math.isfinite(s.z)]

    def ebrem_steps(self) -> List[StepRecord]:
        return [s for s in self.steps if s.process == "eBrem"]

    def primary_ebrem_steps(self) -> List[StepRecord]:
        return [s for s in self.steps if is_primary_step(s) and s.process == "eBrem"]

    def endpoint_retained(self) -> List[float]:
        return [e.endpoint_retained_primary for e in self.events if math.isfinite(e.endpoint_retained_primary)]

    def endpoint_r(self) -> List[float]:
        return [e.endpoint_r_primary for e in self.events if math.isfinite(e.endpoint_r_primary)]

    def event_loss_all(self) -> List[float]:
        return [e.loss_all for e in self.events]

    def event_loss_primary(self) -> List[float]:
        return [e.loss_primary for e in self.events]

    def event_loss_ebrem_primary(self) -> List[float]:
        return [e.loss_ebrem_primary for e in self.events]

    def event_tx0_primary(self) -> List[float]:
        return [e.tx0_primary for e in self.events]


def is_primary_step(step: StepRecord) -> bool:
    return step.parent_id == 0


def parse_sample(text: str) -> Tuple[str, str]:
    if "=" in text:
        label, path = text.split("=", 1)
    else:
        path = text
        label = os.path.splitext(os.path.basename(path))[0]
    label = label.strip()
    path = path.strip()
    if not label or not path:
        raise argparse.ArgumentTypeError("samples must be LABEL=FILE or FILE")
    return label, path


def as_list(obj: Iterable) -> List:
    return [x for x in obj]


def quantile(values: Sequence[float], p: float) -> float:
    clean = sorted(float(v) for v in values if math.isfinite(float(v)))
    if not clean:
        return float("nan")
    x = p * (len(clean) - 1)
    lo = int(math.floor(x))
    hi = int(math.ceil(x))
    if lo == hi:
        return clean[lo]
    return clean[lo] + (clean[hi] - clean[lo]) * (x - lo)


def mean(values: Sequence[float]) -> float:
    clean = [float(v) for v in values if math.isfinite(float(v))]
    return sum(clean) / len(clean) if clean else float("nan")


def safe_div(num: float, den: float) -> float:
    return num / den if abs(den) > EPS else float("nan")


def branch_names(tree) -> set:
    return {b.GetName() for b in tree.GetListOfBranches()}


def read_sample(label: str, filename: str, max_events: int = 0) -> SampleSummary:
    f = ROOT.TFile.Open(filename)
    if not f or f.IsZombie():
        raise RuntimeError(f"cannot open {filename}")
    tree = f.Get("g4step_tuple")
    if not tree:
        raise RuntimeError(f"no g4step_tuple tree in {filename}")

    branches = branch_names(tree)
    required = {"event_id", "pre_p", "post_p", "pre_r", "post_r", "post_z", "loss", "retained", "step_tX0", "process", "material"}
    missing = sorted(required - branches)
    if missing:
        raise RuntimeError(f"{filename}: missing required branches: {', '.join(missing)}")

    summary = SampleSummary(label=label, filename=filename, entries=int(tree.GetEntries()))
    nentries = int(tree.GetEntries()) if max_events <= 0 else min(int(tree.GetEntries()), max_events)

    for iev in range(nentries):
        tree.GetEntry(iev)
        event_id = int(getattr(tree, "event_id", iev))
        pre_p = as_list(tree.pre_p)
        post_p = as_list(tree.post_p)
        pre_r = as_list(tree.pre_r)
        post_r = as_list(tree.post_r)
        post_z = as_list(tree.post_z)
        losses = as_list(tree.loss)
        retained = as_list(tree.retained)
        tx0 = as_list(tree.step_tX0)
        processes = [str(x) for x in tree.process]
        materials = [str(x) for x in tree.material]
        n = len(pre_p)

        track_ids = as_list(tree.track_id) if "track_id" in branches else [0] * n
        parent_ids = as_list(tree.parent_id) if "parent_id" in branches else [0] * n
        pdgs = as_list(tree.pdg) if "pdg" in branches else [0] * n

        event_steps = []
        for i in range(n):
            pre = float(pre_p[i])
            post = float(post_p[i])
            z = float(retained[i]) if i < len(retained) else safe_div(post, pre)
            loss = float(losses[i]) if i < len(losses) else pre - post
            step = StepRecord(
                event_id=event_id,
                index=i,
                track_id=int(track_ids[i]),
                parent_id=int(parent_ids[i]),
                pdg=int(pdgs[i]),
                pre_p=pre,
                post_p=post,
                pre_r=float(pre_r[i]),
                post_r=float(post_r[i]),
                post_z=float(post_z[i]),
                z=z,
                loss=loss,
                step_tx0=float(tx0[i]),
                process=processes[i],
                material=materials[i],
            )
            summary.steps.append(step)
            event_steps.append(step)
            summary.process_counts[step.process] += 1
            summary.material_counts[step.material] += 1

        primary = [s for s in event_steps if is_primary_step(s)] or event_steps
        start_step = min(primary, key=lambda s: s.pre_r) if primary else None
        endpoint_step = max(primary, key=lambda s: s.post_r) if primary else None
        first_p = start_step.pre_p if start_step else float("nan")
        last_p = endpoint_step.post_p if endpoint_step else float("nan")
        endpoint = safe_div(last_p, first_p)
        endpoint_r = endpoint_step.post_r if endpoint_step else float("nan")
        endpoint_z = endpoint_step.post_z if endpoint_step else float("nan")
        summary.events.append(
            EventRecord(
                event_id=event_id,
                n_all=len(event_steps),
                n_primary=len(primary),
                loss_all=sum(s.loss for s in event_steps),
                loss_primary=sum(s.loss for s in primary),
                loss_ebrem_primary=sum(s.loss for s in primary if s.process == "eBrem"),
                tx0_all=sum(s.step_tx0 for s in event_steps),
                tx0_primary=sum(s.step_tx0 for s in primary),
                endpoint_retained_primary=endpoint,
                endpoint_r_primary=endpoint_r,
                endpoint_z_primary=endpoint_z,
                first_primary_p=first_p,
                last_primary_p=last_p,
            )
        )

    f.Close()
    return summary


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def write_event_csv(samples: Sequence[SampleSummary], outdir: str) -> None:
    path = os.path.join(outdir, "event_summary.csv")
    with open(path, "w", newline="") as fp:
        w = csv.writer(fp)
        w.writerow([
            "sample", "event_id", "n_steps_all", "n_steps_primary", "endpoint_retained_primary",
            "endpoint_r_primary_mm", "endpoint_z_primary_mm",
            "first_primary_p_GeV", "last_primary_p_GeV", "loss_all_GeV", "loss_primary_GeV",
            "loss_primary_eBrem_GeV", "tx0_all", "tx0_primary",
        ])
        for s in samples:
            for e in s.events:
                w.writerow([
                    s.label, e.event_id, e.n_all, e.n_primary, e.endpoint_retained_primary,
                    e.endpoint_r_primary, e.endpoint_z_primary,
                    e.first_primary_p, e.last_primary_p, e.loss_all, e.loss_primary,
                    e.loss_ebrem_primary, e.tx0_all, e.tx0_primary,
                ])


def write_step_csv(samples: Sequence[SampleSummary], outdir: str) -> None:
    path = os.path.join(outdir, "step_summary.csv")
    with open(path, "w", newline="") as fp:
        w = csv.writer(fp)
        w.writerow([
            "sample", "event_id", "step_index", "track_id", "parent_id", "pdg", "is_primary",
            "process", "material", "pre_p_GeV", "post_p_GeV", "pre_r_mm", "post_r_mm", "post_z_mm",
            "z_post_over_pre", "loss_GeV", "step_tX0",
        ])
        for s in samples:
            for step in s.steps:
                w.writerow([
                    s.label, step.event_id, step.index, step.track_id, step.parent_id, step.pdg,
                    int(is_primary_step(step)), step.process, step.material, step.pre_p, step.post_p,
                    step.pre_r, step.post_r, step.post_z, step.z, step.loss, step.step_tx0,
                ])


def write_count_csv(samples: Sequence[SampleSummary], outdir: str, attr: str) -> None:
    path = os.path.join(outdir, f"{attr}_counts.csv")
    with open(path, "w", newline="") as fp:
        w = csv.writer(fp)
        w.writerow(["sample", attr, "count", "fraction"])
        for s in samples:
            counts = getattr(s, f"{attr}_counts")
            total = sum(counts.values())
            for key, count in counts.most_common():
                w.writerow([s.label, key, count, safe_div(count, total)])


def values_for_plot(sample: SampleSummary, quantity: str) -> List[float]:
    if quantity == "endpoint":
        return sample.endpoint_retained()
    if quantity == "endpoint_r":
        return sample.endpoint_r()
    if quantity == "event_loss_all":
        return sample.event_loss_all()
    if quantity == "event_loss_primary":
        return sample.event_loss_primary()
    if quantity == "event_loss_ebrem_primary":
        return sample.event_loss_ebrem_primary()
    if quantity == "all_z":
        return sample.all_z()
    if quantity == "primary_z":
        return sample.primary_z()
    if quantity == "all_loss":
        return sample.all_loss()
    if quantity == "ebrem_loss":
        return [s.loss for s in sample.ebrem_steps()]
    if quantity == "primary_ebrem_z":
        return [s.z for s in sample.primary_ebrem_steps()]
    raise ValueError(quantity)


def make_hist(name: str, title: str, values: Sequence[float], bins: int, xmin: float, xmax: float):
    h = ROOT.TH1D(name, title, bins, xmin, xmax)
    h.Sumw2()
    for value in values:
        if math.isfinite(float(value)):
            h.Fill(float(value))
    if h.Integral() > 0:
        h.Scale(1.0 / h.Integral())
    return h


def draw_overlay(samples: Sequence[SampleSummary], outdir: str, stem: str, quantity: str, title: str,
                 xtitle: str, bins: int, xmin: float, xmax: float, logy: bool = False) -> None:
    canvas = ROOT.TCanvas(f"c_{stem}", stem, 900, 700)
    canvas.SetLeftMargin(0.12)
    canvas.SetRightMargin(0.05)
    canvas.SetLogy(logy)
    legend = ROOT.TLegend(0.62, 0.70, 0.90, 0.90)
    colors = [ROOT.kBlue + 1, ROOT.kRed + 1, ROOT.kGreen + 2, ROOT.kMagenta + 2, ROOT.kOrange + 7]
    max_y = 0.0
    hists = []
    for idx, sample in enumerate(samples):
        vals = values_for_plot(sample, quantity)
        hist = make_hist(f"h_{stem}_{idx}", title, vals, bins, xmin, xmax)
        hist.SetLineColor(colors[idx % len(colors)])
        hist.SetLineWidth(2)
        hist.GetXaxis().SetTitle(xtitle)
        hist.GetYaxis().SetTitle("normalized entries")
        max_y = max(max_y, hist.GetMaximum())
        hists.append(hist)
        legend.AddEntry(hist, f"{sample.label} (n={len(vals)})", "l")
    if not hists:
        return
    hists[0].SetMaximum(max_y * (20.0 if logy else 1.25) if max_y > 0 else 1.0)
    if logy:
        hists[0].SetMinimum(1.0e-5)
    hists[0].Draw("hist")
    for hist in hists[1:]:
        hist.Draw("hist same")
    legend.Draw()
    for ext in ("png", "pdf"):
        canvas.SaveAs(os.path.join(outdir, f"{stem}.{ext}"))


def draw_loss_vs_tx0(samples: Sequence[SampleSummary], outdir: str) -> None:
    for sample in samples:
        canvas = ROOT.TCanvas(f"c_loss_tx0_{sample.label}", sample.label, 900, 750)
        canvas.SetLeftMargin(0.12)
        canvas.SetRightMargin(0.15)
        canvas.SetLogz(True)
        hist = ROOT.TH2D(
            f"h_loss_tx0_{sample.label}",
            f"{sample.label}: single-step loss vs t/X0;step t/X0;loss [GeV]",
            120, 0.0, 0.005, 120, 0.0, 0.2,
        )
        for step in sample.steps:
            if step.loss > 0 and step.step_tx0 >= 0:
                hist.Fill(step.step_tx0, step.loss)
        hist.Draw("colz")
        for ext in ("png", "pdf"):
            canvas.SaveAs(os.path.join(outdir, f"loss_vs_step_tX0_{sample.label}.{ext}"))


def write_summary(samples: Sequence[SampleSummary], outdir: str) -> None:
    path = os.path.join(outdir, "summary.txt")
    with open(path, "w") as fp:
        fp.write("G4 step tuple analysis summary\n")
        fp.write("truth source: GsfMaterialStepRecorderAnaElemTool / g4step_tuple\n\n")
        for s in samples:
            primary_steps = s.primary_steps()
            ebrem_steps = s.ebrem_steps()
            primary_ebrem_steps = s.primary_ebrem_steps()
            fp.write(f"[{s.label}]\n")
            fp.write(f"file {s.filename}\n")
            fp.write(f"tree_entries_read {len(s.events)} original_entries {s.entries}\n")
            fp.write(f"steps_all {len(s.steps)} steps_primary {len(primary_steps)} steps_eBrem {len(ebrem_steps)} steps_primary_eBrem {len(primary_ebrem_steps)}\n")
            fp.write(f"mean_steps_per_event_all {safe_div(len(s.steps), len(s.events)):.8g}\n")
            fp.write(f"mean_event_endpoint_retained_primary {mean(s.endpoint_retained()):.8g}\n")
            fp.write("endpoint_retained_primary_q01_q05_q10_q50_q90_q95_q99 " + " ".join(f"{quantile(s.endpoint_retained(), q):.8g}" for q in (0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99)) + "\n")
            fp.write("endpoint_r_primary_mm_q01_q05_q10_q50_q90_q95_q99 " + " ".join(f"{quantile(s.endpoint_r(), q):.8g}" for q in (0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99)) + "\n")
            fp.write(f"mean_event_loss_all_GeV {mean(s.event_loss_all()):.8g}\n")
            fp.write(f"mean_event_loss_primary_GeV {mean(s.event_loss_primary()):.8g}\n")
            fp.write(f"mean_event_loss_primary_eBrem_GeV {mean(s.event_loss_ebrem_primary()):.8g}\n")
            fp.write("event_loss_primary_q50_q90_q95_q99 " + " ".join(f"{quantile(s.event_loss_primary(), q):.8g}" for q in (0.50, 0.90, 0.95, 0.99)) + "\n")
            fp.write(f"mean_event_tX0_primary {mean(s.event_tx0_primary()):.8g}\n")
            fp.write("single_step_z_all_q01_q05_q10_q50_q90_q95_q99 " + " ".join(f"{quantile(s.all_z(), q):.8g}" for q in (0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99)) + "\n")
            fp.write("single_step_z_primary_eBrem_q01_q05_q10_q50_q90_q95_q99 " + " ".join(f"{quantile([x.z for x in primary_ebrem_steps], q):.8g}" for q in (0.01, 0.05, 0.10, 0.50, 0.90, 0.95, 0.99)) + "\n")
            for threshold in TAIL_THRESHOLDS:
                fp.write(f"tail_all_P_z_lt_{threshold:g} {mean([1.0 if z < threshold else 0.0 for z in s.all_z()]):.8g}\n")
                fp.write(f"tail_primary_eBrem_P_z_lt_{threshold:g} {mean([1.0 if step.z < threshold else 0.0 for step in primary_ebrem_steps]):.8g}\n")
            for threshold in LOSS_THRESHOLDS_GEV:
                fp.write(f"tail_all_P_loss_gt_{threshold:g}_GeV {mean([1.0 if x > threshold else 0.0 for x in s.all_loss()]):.8g}\n")
                fp.write(f"tail_primary_eBrem_P_loss_gt_{threshold:g}_GeV {mean([1.0 if step.loss > threshold else 0.0 for step in primary_ebrem_steps]):.8g}\n")
            fp.write("process_counts " + " ".join(f"{k}:{v}" for k, v in s.process_counts.most_common()) + "\n")
            fp.write("top_materials " + " ".join(f"{k}:{v}" for k, v in s.material_counts.most_common(20)) + "\n\n")

        if len(samples) >= 2:
            base = samples[0]
            fp.write("[ratios relative_to_first_sample]\n")
            for s in samples[1:]:
                fp.write(f"{base.label}_over_{s.label}_mean_event_loss_primary {safe_div(mean(base.event_loss_primary()), mean(s.event_loss_primary())):.8g}\n")
                fp.write(f"{base.label}_over_{s.label}_mean_event_endpoint_loss {safe_div(1.0 - mean(base.endpoint_retained()), 1.0 - mean(s.endpoint_retained())):.8g}\n")
                fp.write(f"{base.label}_over_{s.label}_mean_event_tX0_primary {safe_div(mean(base.event_tx0_primary()), mean(s.event_tx0_primary())):.8g}\n")


def make_plots(samples: Sequence[SampleSummary], outdir: str) -> None:
    draw_overlay(samples, outdir, "event_endpoint_retained_primary", "endpoint", "Primary endpoint retained momentum", "outermost primary post_p / innermost primary pre_p", 120, 0.0, 1.02, True)
    draw_overlay(samples, outdir, "event_endpoint_r_primary", "endpoint_r", "Primary endpoint radius", "outermost primary post_r [mm]", 120, 0.0, 260.0, False)
    draw_overlay(samples, outdir, "event_cumulative_loss_all", "event_loss_all", "Event cumulative momentum loss", "sum step loss [GeV]", 120, 0.0, 0.25, True)
    draw_overlay(samples, outdir, "event_cumulative_loss_primary", "event_loss_primary", "Primary-track cumulative momentum loss", "sum primary step loss [GeV]", 120, 0.0, 0.25, True)
    draw_overlay(samples, outdir, "single_step_z_all", "all_z", "Single-step retained momentum", "z = post_p / pre_p", 120, 0.0, 1.02, True)
    draw_overlay(samples, outdir, "single_step_loss_tail_all", "all_loss", "Single-step momentum-loss tail", "pre_p - post_p [GeV]", 120, 0.0, 0.20, True)
    draw_overlay(samples, outdir, "single_step_z_primary_eBrem", "primary_ebrem_z", "Primary eBrem retained momentum", "z = post_p / pre_p", 120, 0.0, 1.02, True)
    draw_overlay(samples, outdir, "single_step_loss_eBrem", "ebrem_loss", "eBrem single-step momentum loss", "pre_p - post_p [GeV]", 120, 0.0, 0.20, True)
    draw_loss_vs_tx0(samples, outdir)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sample", action="append", type=parse_sample, required=True,
                        help="Input sample as LABEL=FILE. Repeat for comparisons.")
    parser.add_argument("--outdir", default="G4MaterialStepComparison/analysis_g4step",
                        help="Output directory for summaries and plots.")
    parser.add_argument("--max-events", type=int, default=0,
                        help="Optional event limit for smoke tests.")
    args = parser.parse_args()

    ensure_dir(args.outdir)
    samples = [read_sample(label, path, args.max_events) for label, path in args.sample]
    write_summary(samples, args.outdir)
    write_event_csv(samples, args.outdir)
    write_step_csv(samples, args.outdir)
    write_count_csv(samples, args.outdir, "process")
    write_count_csv(samples, args.outdir, "material")
    make_plots(samples, args.outdir)

    print(f"Wrote G4-step analysis outputs to {args.outdir}")
    for sample in samples:
        print(f"{sample.label}: events={len(sample.events)} steps={len(sample.steps)} primary_eBrem={len(sample.primary_ebrem_steps())}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
