#!/usr/bin/env python3
"""Find the first inward posterior crossing of radiative over identity."""

from __future__ import annotations

import argparse
import math
import re
from pathlib import Path


EVENT = re.compile(r"GSF event index (\d+)")
MIX = re.compile(r"MIX reverse-posterior/norm\s+hit=\s*(-?\d+)")
COMP = re.compile(
    r"comp\[\d+\] id=(\d+).*?noRad=(\d+).*?w=([0-9.eE+-]+).*?pT=([0-9.eE+-]+)")
SIGNATURE = re.compile(r"signatures forward=.*? reverse=(.*)$")
UPDATE = re.compile(
    r"REVERSE UPDATE accept hit=(\d+) id=(\d+) pT=([0-9.eE+-]+) "
    r"priorWeight=([0-9.eE+-]+) dchi2=([0-9.eE+-]+) logDetS=([0-9.eE+-]+)")
EXACT = re.compile(r"reverse-exact-measurement: predicted=.*? residual=(.*?) H=.*? S=(.*)$")


def scan(path: Path, entry: int) -> list[dict]:
    stages: dict[int, list[dict]] = {}
    updates: dict[tuple[int, int], dict] = {}
    active_hit = None
    current = None
    last_update = None
    in_event = False
    with path.open(errors="replace") as stream:
        for line in stream:
            event = EVENT.search(line)
            if event:
                in_event = int(event.group(1)) == entry
                active_hit = current = last_update = None
                continue
            if not in_event:
                continue
            update = UPDATE.search(line)
            if update:
                hit, component_id = int(update.group(1)), int(update.group(2))
                updates[(hit, component_id)] = {
                    "pt": float(update.group(3)),
                    "prior_weight": float(update.group(4)),
                    "dchi2": float(update.group(5)),
                    "log_det_s": float(update.group(6)),
                }
                last_update = (hit, component_id)
                continue
            exact = EXACT.search(line)
            if exact and last_update in updates:
                updates[last_update]["residual"] = exact.group(1)
                updates[last_update]["innovation_covariance"] = exact.group(2)
                last_update = None
                continue
            mix = MIX.search(line)
            if mix:
                active_hit = int(mix.group(1))
                stages[active_hit] = []
                current = None
                continue
            if " MIX " in line:
                active_hit = current = None
            component = COMP.search(line)
            if component and active_hit is not None:
                current = {
                    "id": int(component.group(1)),
                    "no_rad": int(component.group(2)),
                    "posterior_weight": float(component.group(3)),
                    "posterior_pt": float(component.group(4)),
                    "signature": "",
                }
                stages[active_hit].append(current)
                continue
            signature = SIGNATURE.search(line)
            if signature and current is not None:
                current["signature"] = signature.group(1).strip()

    crossings = []
    for hit in sorted(stages, reverse=True):
        items = stages[hit]
        identities = [item for item in items if item["no_rad"]]
        radiatives = [item for item in items if not item["no_rad"]]
        if not identities or not radiatives:
            continue
        identity = max(identities, key=lambda item: item["posterior_weight"])
        radiative = max(radiatives, key=lambda item: item["posterior_weight"])
        if radiative["posterior_weight"] <= identity["posterior_weight"]:
            continue
        rad_update = updates[(hit, radiative["id"])]
        id_update = updates[(hit, identity["id"])]
        log_lr = -0.5 * (
            rad_update["dchi2"] + rad_update["log_det_s"] -
            id_update["dchi2"] - id_update["log_det_s"])
        crossings.append({
            "hit": hit, "radiative": {**radiative, **rad_update},
            "identity": {**identity, **id_update},
            "prior_odds": rad_update["prior_weight"] / id_update["prior_weight"],
            "likelihood_ratio": math.exp(log_lr),
            "posterior_odds": (radiative["posterior_weight"] /
                               identity["posterior_weight"]),
        })
    return crossings


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--case", action="append", nargs=3, required=True,
                        metavar=("LABEL", "LOG", "ENTRY"))
    args = parser.parse_args()
    for label, path, entry in args.case:
        crossings = scan(Path(path), int(entry))
        if not crossings:
            print(f"{label}: no radiative-over-identity posterior crossing")
            continue
        item = crossings[0]
        print(f"{label}: first crossing hit={item['hit']} "
              f"prior/L/post={item['prior_odds']:.6g}/"
              f"{item['likelihood_ratio']:.6g}/{item['posterior_odds']:.6g}")
        for name in ("identity", "radiative"):
            state = item[name]
            print(f"  {name}: pT={state['pt']:.6g} priorW={state['prior_weight']:.6g} "
                  f"postW={state['posterior_weight']:.6g} "
                  f"dchi2={state['dchi2']:.6g} logDetS={state['log_det_s']:.6g}")
            print(f"    residual={state.get('residual', '')} "
                  f"S={state.get('innovation_covariance', '')}")
            print(f"    signature={state['signature']}")


if __name__ == "__main__":
    main()
