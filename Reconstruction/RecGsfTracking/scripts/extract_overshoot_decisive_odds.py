#!/usr/bin/env python3
"""Extract decisive-hit prior and likelihood odds for overshoot traces."""

import argparse
import csv
import math
import re
from pathlib import Path


MIX = re.compile(r"MIX (reverse-pre-reduce|reverse-post-reduce/norm|reverse-after-hit)\s+hit=\s*(\d+)")
COMP = re.compile(r"top\d+\s+comp\[\d+\] id=(\d+).*?"
                  r"(?:procH=(-?\d+) procG=(-?\d+) procF=([0-9.eE+-]+) )?"
                  r"w=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
UPDATE = re.compile(r"REVERSE UPDATE accept hit=(\d+) id=(\d+) pT=([0-9.eE+-]+)"
                    r" dchi2=([0-9.eE+-]+) logDetS=([0-9.eE+-]+)")


def parse_log(path, hit):
    stages = {"reverse-pre-reduce": [], "reverse-post-reduce/norm": [],
              "reverse-after-hit": [],
              "previous-after-hit": []}
    updates = {}
    active = None
    with path.open(errors="replace") as stream:
        for line in stream:
            if "GSF Track 01" in line:
                break
            if " MIX " in line:
                match = MIX.search(line)
                active = None
                if match:
                    stage, stage_hit = match.group(1), int(match.group(2))
                    if stage_hit == hit:
                        active = stage
                    elif stage == "reverse-after-hit" and stage_hit == hit + 1:
                        active = "previous-after-hit"
                continue
            match = COMP.search(line)
            if active and match:
                stages[active].append({"id": int(match.group(1)),
                                       "process_hit": int(match.group(2))
                                           if match.group(2) is not None else -1,
                                       "process_component": int(match.group(3))
                                           if match.group(3) is not None else -1,
                                       "process_fraction": float(match.group(4))
                                           if match.group(4) is not None else 1.0,
                                       "weight": float(match.group(5)),
                                       "pt": float(match.group(6))})
                continue
            match = UPDATE.search(line)
            if match and int(match.group(1)) == hit:
                updates[int(match.group(2))] = {
                    "pt": float(match.group(3)),
                    "dchi2": float(match.group(4)),
                    "log_det_s": float(match.group(5)),
                }
    return stages, updates


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trajectories", type=Path, required=True)
    parser.add_argument("--log-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    trajectories = list(csv.DictReader(args.trajectories.open(newline="")))
    output = []
    for row in trajectories:
        seed, entry = int(row["seed"]), int(row["entry"])
        hit, truth = int(row["largest_jump_hit"]), float(row["truth_pt"])
        stages, updates = parse_log(args.log_dir / f"{seed}-{entry}.log", hit)
        prior_items = (stages["reverse-post-reduce/norm"] or
                       stages["previous-after-hit"])
        prior = {item["id"]: item for item in prior_items}
        pre_reduce = {item["id"]: item for item in stages["reverse-pre-reduce"]}
        posterior = stages["reverse-after-hit"]
        if not prior or not posterior:
            raise RuntimeError(f"missing decisive state for {seed}/{entry} hit {hit}")
        winner = max(posterior, key=lambda item: item["weight"])
        compatible_candidates = [item for item in posterior if item["id"] != winner["id"]]
        compatible = min(compatible_candidates,
                         key=lambda item: abs(item["pt"] - truth))
        if winner["id"] not in prior or compatible["id"] not in prior:
            raise RuntimeError(f"missing prior IDs for {seed}/{entry}")
        winner_update, compatible_update = updates[winner["id"]], updates[compatible["id"]]
        winner_pre_reduce = pre_reduce.get(winner["id"], prior[winner["id"]])
        compatible_pre_reduce = pre_reduce.get(compatible["id"], prior[compatible["id"]])
        log_likelihood_ratio = -0.5 * (
            winner_update["dchi2"] + winner_update["log_det_s"] -
            compatible_update["dchi2"] - compatible_update["log_det_s"])
        prior_odds = prior[winner["id"]]["weight"] / prior[compatible["id"]]["weight"]
        likelihood_ratio = math.exp(min(700.0, max(-700.0, log_likelihood_ratio)))
        output.append({
            "seed": seed, "entry": entry, "decisive_hit": hit,
            "truth_pt": truth,
            "winner_id": winner["id"], "winner_prior_weight": prior[winner["id"]]["weight"],
            "winner_pre_reduce_weight": winner_pre_reduce["weight"],
            "winner_kl_weight_amplification": prior[winner["id"]]["weight"] /
                winner_pre_reduce["weight"],
            "winner_kl_pt_shift": prior[winner["id"]]["pt"] - winner_pre_reduce["pt"],
            "winner_preupdate_pt": prior[winner["id"]]["pt"],
            "winner_process_hit": prior[winner["id"]]["process_hit"],
            "winner_process_component": prior[winner["id"]]["process_component"],
            "winner_process_fraction": prior[winner["id"]]["process_fraction"],
            "winner_postupdate_pt": winner["pt"],
            "winner_dchi2": winner_update["dchi2"],
            "winner_log_det_s": winner_update["log_det_s"],
            "compatible_id": compatible["id"],
            "compatible_prior_weight": prior[compatible["id"]]["weight"],
            "compatible_pre_reduce_weight": compatible_pre_reduce["weight"],
            "compatible_kl_weight_amplification": prior[compatible["id"]]["weight"] /
                compatible_pre_reduce["weight"],
            "compatible_kl_pt_shift": prior[compatible["id"]]["pt"] - compatible_pre_reduce["pt"],
            "compatible_preupdate_pt": prior[compatible["id"]]["pt"],
            "compatible_process_hit": prior[compatible["id"]]["process_hit"],
            "compatible_process_component": prior[compatible["id"]]["process_component"],
            "compatible_process_fraction": prior[compatible["id"]]["process_fraction"],
            "compatible_postupdate_pt": compatible["pt"],
            "compatible_dchi2": compatible_update["dchi2"],
            "compatible_log_det_s": compatible_update["log_det_s"],
            "prior_odds_winner_to_compatible": prior_odds,
            "log_likelihood_ratio_winner_to_compatible": log_likelihood_ratio,
            "likelihood_ratio_winner_to_compatible": likelihood_ratio,
            "posterior_odds_winner_to_compatible": winner["weight"] / compatible["weight"],
        })
    output.sort(key=lambda row: (row["decisive_hit"], row["seed"], row["entry"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(output[0]))
        writer.writeheader()
        writer.writerows(output)
    print(args.output.read_text(), end="")


if __name__ == "__main__":
    main()
