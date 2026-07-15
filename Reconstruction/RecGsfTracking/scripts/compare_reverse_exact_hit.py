#!/usr/bin/env python3
"""Compare exact identity and g2 posteriors at one reverse measurement hit."""

from __future__ import annotations

import argparse
import math
import re
from pathlib import Path


EVENT = re.compile(r"GSF event index (\d+)")
UPDATE = re.compile(
    r"REVERSE UPDATE accept hit=(\d+) id=(\d+) pT=([0-9.eE+-]+) "
    r"priorWeight=([0-9.eE+-]+) dchi2=([0-9.eE+-]+) "
    r"logDetS=([0-9.eE+-]+)")
EXACT = re.compile(
    r"reverse-exact-measurement: predicted=(.*?) residual=(.*?) H=.*? R=(.*?) S=(.*)$")
MIX = re.compile(r"MIX reverse-posterior/norm\s+hit=\s*(-?\d+)")
COMP = re.compile(
    r"comp\[\d+\] id=(\d+).*?noRad=(\d+) "
    r"procH=(-?\d+) procG=(-?\d+) procF=([0-9.eE+-]+) "
    r"w=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
PT = re.compile(r"\bpT\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)\s+GeV")


def parse(path: Path, entry: int, hit: int) -> dict:
    updates: dict[int, dict] = {}
    posterior: dict[int, dict] = {}
    active_mix = False
    in_event = False
    last_update = None
    pt = None
    with path.open(errors="replace") as stream:
        for line in stream:
            event = EVENT.search(line)
            if event:
                in_event = int(event.group(1)) == entry
                active_mix = False
                last_update = None
                continue
            if not in_event:
                continue
            update = UPDATE.search(line)
            if update:
                update_hit, component_id = int(update.group(1)), int(update.group(2))
                last_update = component_id if update_hit == hit else None
                if last_update is not None:
                    updates[component_id] = {
                        "pt": float(update.group(3)),
                        "prior_weight": float(update.group(4)),
                        "dchi2": float(update.group(5)),
                        "log_det_s": float(update.group(6)),
                    }
                continue
            exact = EXACT.search(line)
            if exact and last_update is not None:
                updates[last_update].update({
                    "predicted": exact.group(1),
                    "residual": exact.group(2),
                    "measurement_covariance": exact.group(3),
                    "innovation_covariance": exact.group(4),
                })
                last_update = None
                continue
            mix = MIX.search(line)
            if mix:
                active_mix = int(mix.group(1)) == hit
                continue
            if " MIX " in line:
                active_mix = False
            component = COMP.search(line)
            if component and active_mix:
                posterior[int(component.group(1))] = {
                    "no_rad": int(component.group(2)),
                    "process_hit": int(component.group(3)),
                    "process_mode": int(component.group(4)),
                    "process_fraction": float(component.group(5)),
                    "posterior_weight": float(component.group(6)),
                    "posterior_pt": float(component.group(7)),
                }
                continue
            table = PT.search(line)
            if table:
                pt = tuple(map(float, table.groups()))
    candidates = {component_id: {**item, **updates[component_id]}
                  for component_id, item in posterior.items()
                  if component_id in updates}
    identities = [item for item in candidates.values() if item["no_rad"]]
    g2s = [item for item in candidates.values()
           if item["process_hit"] == hit and item["process_mode"] == 2]
    if not identities or not g2s or pt is None:
        raise RuntimeError(f"missing identity/g2/pt for event {entry}, hit {hit}")
    identity = max(identities, key=lambda item: item["posterior_weight"])
    g2 = max(g2s, key=lambda item: item["posterior_weight"])
    log_lr = -0.5 * (
        g2["dchi2"] + g2["log_det_s"] -
        identity["dchi2"] - identity["log_det_s"])
    return {
        "truth_pt": pt[0], "lcio_pt": pt[1], "gsf_pt": pt[2],
        "identity": identity, "g2": g2,
        "prior_odds_g2_to_identity": g2["prior_weight"] / identity["prior_weight"],
        "likelihood_ratio_g2_to_identity": math.exp(log_lr),
        "posterior_odds_g2_to_identity": (
            g2["posterior_weight"] / identity["posterior_weight"]),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", nargs=4, required=True,
                        metavar=("LABEL", "LOG", "ENTRY", "HIT"))
    args = parser.parse_args()
    for label, path, entry, hit in args.case:
        result = parse(Path(path), int(entry), int(hit))
        identity, g2 = result["identity"], result["g2"]
        print(f"{label} Truth/LCIO/GSF={result['truth_pt']:.5f}/"
              f"{result['lcio_pt']:.5f}/{result['gsf_pt']:.5f}")
        print(f"  odds prior/L/post={result['prior_odds_g2_to_identity']:.6g}/"
              f"{result['likelihood_ratio_g2_to_identity']:.6g}/"
              f"{result['posterior_odds_g2_to_identity']:.6g}")
        for name, item in (("identity", identity), ("g2", g2)):
            print(f"  {name}: priorW={item['prior_weight']:.6g} "
                  f"postW={item['posterior_weight']:.6g} pT={item['pt']:.6g} "
                  f"dchi2={item['dchi2']:.6g} logDetS={item['log_det_s']:.6g} "
                  f"f={item['process_fraction']:.6g}")
            print(f"    residual={item['residual']} S={item['innovation_covariance']}")


if __name__ == "__main__":
    main()
