#!/usr/bin/env python3
"""Extract state-level diagnostics for the positive-LCIO amplification set."""

from __future__ import annotations

import argparse
import csv
import math
import re
from pathlib import Path


EVENT = re.compile(r"GSF event index (\d+)")
MIX = re.compile(r"MIX (reverse-start|reverse-pre-reduce|reverse-post-reduce/norm|reverse-after-hit)\s+hit=\s*(-?\d+)")
COMP = re.compile(
    r"top\d+\s+comp\[\d+\] id=(\d+).*?noRad=(\d+) "
    r"procH=(-?\d+) procG=(-?\d+) procF=([0-9.eE+-]+) "
    r"w=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
UPDATE = re.compile(
    r"REVERSE UPDATE accept hit=(\d+) id=(\d+) pT=([0-9.eE+-]+) "
    r"dchi2=([0-9.eE+-]+).*?logDetS=([0-9.eE+-]+)")
IP = re.compile(r"REVERSE IP output:.*?bestId=(\d+) bestWeight=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
SIGNATURE = re.compile(r"REVERSE SELECTED process-signature=(.*)$")
PT_TABLE = re.compile(r"\bpT\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)\s+GeV")


def component(match: re.Match[str]) -> dict:
    return {
        "id": int(match.group(1)), "no_rad": int(match.group(2)),
        "process_hit": int(match.group(3)),
        "process_mode": int(match.group(4)),
        "process_fraction": float(match.group(5)),
        "weight": float(match.group(6)), "pt": float(match.group(7)),
    }


def parse_event(path: Path, entry: int) -> dict:
    stages: dict[tuple[str, int], list[dict]] = {}
    updates: dict[tuple[int, int], dict] = {}
    active_stage = None
    in_event = False
    ip = signature = pt_table = None
    with path.open(errors="replace") as stream:
        for line in stream:
            event = EVENT.search(line)
            if event:
                in_event = int(event.group(1)) == entry
                active_stage = None
                continue
            if not in_event:
                continue
            if " MIX " in line:
                active_stage = None
                mix = MIX.search(line)
                if mix:
                    active_stage = (mix.group(1), int(mix.group(2)))
                    stages[active_stage] = []
                continue
            comp = COMP.search(line)
            if comp and active_stage is not None:
                stages[active_stage].append(component(comp))
                continue
            update = UPDATE.search(line)
            if update:
                updates[(int(update.group(1)), int(update.group(2)))] = {
                    "pt": float(update.group(3)), "dchi2": float(update.group(4)),
                    "log_det_s": float(update.group(5)),
                }
                continue
            found_ip = IP.search(line)
            if found_ip:
                ip = {"id": int(found_ip.group(1)),
                      "weight": float(found_ip.group(2)),
                      "pt": float(found_ip.group(3))}
                continue
            found_signature = SIGNATURE.search(line)
            if found_signature:
                signature = found_signature.group(1).strip()
                continue
            table = PT_TABLE.search(line)
            if table:
                pt_table = tuple(map(float, table.groups()))
    if pt_table is None:
        raise RuntimeError(f"incomplete event {entry} in {path}")
    reverse_output_available = ip is not None
    if signature is None:
        signature = ""
    if ip is None:
        ip = {"id": -1, "weight": math.nan, "pt": pt_table[2]}

    after_hits = sorted((hit, items) for (stage, hit), items in stages.items()
                        if stage == "reverse-after-hit" and hit >= 0)
    decisive = None
    # Compare exact pre-KL posterior scores. The published after-hit mixture
    # may contain new IDs produced by post-update KL merges.
    for hit, items in sorted(after_hits, reverse=True):
        pre_reduce_items = stages.get(("reverse-pre-reduce", hit), [])
        prior_items = (stages.get(("reverse-post-reduce/norm", hit), []) or
                       pre_reduce_items or
                       stages.get(("reverse-after-hit", hit + 1), []) or
                       stages.get(("reverse-start", hit + 1), []))
        candidates = [item for item in prior_items
                      if (hit, item["id"]) in updates]
        radiative = [item for item in candidates if not item["no_rad"]]
        identities = [item for item in candidates if item["no_rad"]]
        if not radiative or not identities:
            continue
        def log_score(item: dict) -> float:
            update = updates[(hit, item["id"])]
            return (math.log(max(item["weight"], 1e-300)) -
                    0.5 * (update["dchi2"] + update["log_det_s"]))
        winner = max(radiative, key=log_score)
        identity = max(identities, key=log_score)
        if log_score(winner) > log_score(identity):
            decisive = (hit, winner, identity, log_score(winner),
                        log_score(identity), pre_reduce_items)
            break
    seed_decisive = decisive is None
    if seed_decisive:
        seed_keys = sorted((hit, items) for (stage, hit), items in stages.items()
                           if stage == "reverse-start")
        hit, prior_items = seed_keys[-1]
        winner = max((item for item in prior_items if not item["no_rad"]),
                     key=lambda item: item["weight"])
        identity = max((item for item in prior_items if item["no_rad"]),
                       key=lambda item: item["weight"])
        winner_score, identity_score = (math.log(max(winner["weight"], 1e-300)),
                                        math.log(max(identity["weight"], 1e-300)))
        pre_reduce_items = prior_items
    else:
        hit, winner, identity, winner_score, identity_score, pre_reduce_items = decisive
        prior_items = (stages.get(("reverse-post-reduce/norm", hit), []) or
                       pre_reduce_items or stages.get(("reverse-after-hit", hit + 1), []) or
                       stages.get(("reverse-start", hit + 1), []))
    prior = {item["id"]: item for item in prior_items}
    pre_reduce = {item["id"]: item for item in pre_reduce_items}
    winner_update = (updates[(hit, winner["id"])] if not seed_decisive else
                     {"pt": winner["pt"], "dchi2": 0.0, "log_det_s": 0.0})
    identity_update = (updates[(hit, identity["id"])] if not seed_decisive else
                       {"pt": identity["pt"], "dchi2": 0.0, "log_det_s": 0.0})
    log_lr = -0.5 * (
        winner_update["dchi2"] + winner_update["log_det_s"] -
        identity_update["dchi2"] - identity_update["log_det_s"])
    posterior_odds = math.exp(max(-700, min(700, winner_score - identity_score)))
    likelihood_ratio = math.exp(max(-700, min(700, log_lr)))
    direct_prior = winner["id"] in prior and identity["id"] in prior
    prior_odds = (prior[winner["id"]]["weight"] /
                  prior[identity["id"]]["weight"] if direct_prior else
                  posterior_odds / likelihood_ratio)
    final_items = dict(after_hits).get(min(hit for hit, _ in after_hits), [])
    final_identities = [item for item in final_items if item["no_rad"]]
    final_identity = (max(final_identities, key=lambda item: item["weight"])
                      if final_identities else
                      {"id": -1, "weight": math.nan, "pt": math.nan})
    radiative_modes = [part for part in signature.split(";")
                       if not re.search(r":g0(?::|$)", part)]
    truth_pt, lcio_pt, tuple_gsf_pt = pt_table

    def kl_amplification(item: dict) -> float:
        if item["id"] not in prior:
            return math.nan
        before = pre_reduce.get(item["id"])
        return (prior[item["id"]]["weight"] / before["weight"]
                if before and before["weight"] else math.nan)

    return {
        "reverse_output_available": int(reverse_output_available),
        "truth_pt": truth_pt, "current_lcio_pt": lcio_pt,
        "current_gsf_pt": ip["pt"], "tuple_gsf_pt": tuple_gsf_pt,
        "current_lcio_residual_pct": 100.0 * (lcio_pt / truth_pt - 1.0),
        "current_gsf_residual_pct": 100.0 * (ip["pt"] / truth_pt - 1.0),
        "current_amplification_pct": 100.0 * ((ip["pt"] - lcio_pt) / truth_pt),
        "selected_id": ip["id"], "selected_weight": ip["weight"],
        "selected_pt": ip["pt"], "selected_bh_surface_modes": ";".join(radiative_modes),
        "selected_process_signature": signature,
        "final_identity_id": final_identity["id"],
        "final_identity_weight": final_identity["weight"],
        "final_identity_pt": final_identity["pt"],
        "decisive_stage": "reverse_seed" if seed_decisive else "measurement",
        "decisive_hit": hit, "radiative_id": winner["id"],
        "radiative_process_hit": winner["process_hit"],
        "radiative_process_mode": winner["process_mode"],
        "radiative_process_fraction": winner["process_fraction"],
        "prior_source": "logged" if direct_prior else "posterior_over_likelihood",
        "radiative_prior_weight": prior[winner["id"]]["weight"] if direct_prior else math.nan,
        "identity_prior_weight": prior[identity["id"]]["weight"] if direct_prior else math.nan,
        "prior_odds_radiative_to_identity": prior_odds,
        "radiative_dchi2": winner_update["dchi2"],
        "radiative_log_det_s": winner_update["log_det_s"],
        "identity_dchi2": identity_update["dchi2"],
        "identity_log_det_s": identity_update["log_det_s"],
        "log_likelihood_ratio_radiative_to_identity": log_lr,
        "likelihood_ratio_radiative_to_identity": likelihood_ratio,
        "posterior_odds_radiative_to_identity": posterior_odds,
        "radiative_kl_weight_amplification": kl_amplification(winner),
        "identity_kl_weight_amplification": kl_amplification(identity),
        "radiative_postupdate_pt": winner_update["pt"],
        "identity_postupdate_pt": identity_update["pt"],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidates", type=Path, required=True)
    parser.add_argument("--log-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    rows = []
    with args.candidates.open(newline="") as stream:
        for source in csv.DictReader(stream):
            seed, entry = int(source["seed"]), int(source["entry"])
            diagnostic = parse_event(args.log_dir / f"seed-{seed}.log", entry)
            row = dict(source)
            row.update(diagnostic)
            if "stored_gsf_residual_pct" in source:
                row["gsf_residual_drift_pct"] = (
                    diagnostic["current_gsf_residual_pct"] -
                    float(source["stored_gsf_residual_pct"]))
            if "stored_amplification_pct" in source:
                row["amplification_drift_pct"] = (
                    diagnostic["current_amplification_pct"] -
                    float(source["stored_amplification_pct"]))
            rows.append(row)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {len(rows)} events to {args.output}")


if __name__ == "__main__":
    main()
