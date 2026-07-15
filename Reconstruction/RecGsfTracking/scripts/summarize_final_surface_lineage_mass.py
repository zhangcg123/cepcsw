#!/usr/bin/env python3
"""Extract final selected-component process-surface marginals from GSF logs."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import re


MIX = re.compile(r"MIX reverse-after-hit\s+hit=\s*0\b")
TOP = re.compile(
    r"top(?P<rank>\d+)\s+.*?id=(?P<id>\d+).*?w=(?P<weight>[-+0-9.eE]+).*?"
    r"pT=(?P<pt>[-+0-9.eE]+)")
MASS = re.compile(r"surface-mode-mass forward=(?P<forward>.*?) reverse=(?P<reverse>.*)$")
SURFACE = re.compile(r"(?P<hit>\d+):\[(?P<body>[^]]*)\]")
MODE = re.compile(r"g(?P<mode>\d+)=(?P<mass>[-+0-9.eE]+)")
RADIATIVE = re.compile(r"(?:^|,)rad=(?P<mass>[-+0-9.eE]+)")


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--logs", type=Path, required=True)
    parser.add_argument("--event-list", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--all-components", action="store_true")
    parser.add_argument(
        "--track-index", type=int, default=0,
        help="Zero-based GSF track within each selected event (default: primary/first)")
    return parser.parse_args()


def final_component_blocks(
        path: Path, track_index: int) -> list[tuple[int, int, float, float, str, str]]:
    lines = path.read_text(errors="replace").splitlines()
    starts = [index for index, line in enumerate(lines) if MIX.search(line)]
    if not starts:
        raise RuntimeError(f"no final reverse mixture in {path}")
    if track_index < 0 or track_index >= len(starts):
        raise RuntimeError(
            f"track index {track_index} outside {len(starts)} reverse mixtures in {path}")
    start = starts[track_index]
    stop = next((index for index in range(start + 1, len(lines))
                 if "REVERSE summary:" in lines[index]), len(lines))
    result = []
    current = None
    for detail in lines[start + 1:stop]:
        top = TOP.search(detail)
        if top:
            current = (int(top.group("rank")), int(top.group("id")),
                       float(top.group("weight")), float(top.group("pt")))
            continue
        mass = MASS.search(detail)
        if mass and current is not None:
            result.append((*current, mass.group("forward"), mass.group("reverse")))
            current = None
    if not result:
        raise RuntimeError(f"no final component surface mass in {path}")
    return result


def parse_surfaces(text: str) -> list[tuple[int, dict[int, float], float]]:
    result = []
    for match in SURFACE.finditer(text):
        body = match.group("body")
        modes = {int(item.group("mode")): float(item.group("mass"))
                 for item in MODE.finditer(body)}
        radiative = RADIATIVE.search(body)
        if radiative is None:
            raise RuntimeError(f"missing radiative mass in {match.group(0)}")
        result.append((int(match.group("hit")), modes,
                       float(radiative.group("mass"))))
    return result


def main() -> None:
    args = arguments()
    events = []
    with args.event_list.open(newline="") as stream:
        events.extend(csv.DictReader(stream))

    fields = ["seed", "entry", "population", "component_rank", "component_id",
              "component_weight", "component_pt", "direction", "hit",
              "radiative_mass", "mode_mass_sum", "no_process_mass"] + [f"g{i}_mass" for i in range(10)]
    rows = []
    for event in events:
        seed = int(event["seed"])
        components = final_component_blocks(
            args.logs / f"seed-{seed}.log", args.track_index)
        if not args.all_components:
            components = [component for component in components
                          if component[0] == 0]
        for rank, component_id, weight, pt, forward, reverse in components:
          for direction, encoded in (("forward", forward), ("reverse", reverse)):
            for hit, modes, radiative in parse_surfaces(encoded):
                row = {
                    "seed": seed,
                    "entry": int(event["entry"]),
                    "population": event.get("population", ""),
                    "component_rank": rank,
                    "component_id": component_id,
                    "component_weight": weight,
                    "component_pt": pt,
                    "direction": direction,
                    "hit": hit,
                    "radiative_mass": radiative,
                    "mode_mass_sum": sum(modes.values()),
                    # A component-local material path can be absent while
                    # another lineage in the eventual aggregate splits here.
                    # Missing mass is therefore explicit no-process mass, not
                    # a normalization failure and not the exact g0 BH atom.
                    "no_process_mass": max(0.0, 1.0 - sum(modes.values())),
                }
                row.update({f"g{i}_mass": modes.get(i, 0.0) for i in range(10)})
                rows.append(row)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    scope = "all final components" if args.all_components else "selected components"
    print(f"wrote {len(rows)} surface marginals for {len(events)} events "
          f"({scope}) to {args.output}")


if __name__ == "__main__":
    main()
