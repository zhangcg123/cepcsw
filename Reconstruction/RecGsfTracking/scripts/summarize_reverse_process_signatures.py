#!/usr/bin/env python3
"""Summarize compact selected reverse-process signatures from focused logs."""

import argparse
import csv
import math
import re
from pathlib import Path


EVENT = re.compile(r"(\d+)-(\d+)\.log$")
SIGNATURE = re.compile(r"REVERSE SELECTED process-signature=(.*)$")
ITEM = re.compile(r"(\d+):g(\d+):f([0-9.eE+-]+)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logs", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    rows = []
    for path in args.logs:
        match_event = EVENT.search(path.name)
        signature = None
        with path.open(errors="replace") as stream:
            for line in stream:
                match = SIGNATURE.search(line)
                if match:
                    signature = match.group(1).strip()
                    break
        if signature is None:
            raise RuntimeError(f"no selected process signature in {path}")
        items = [(int(hit), int(component), float(fraction))
                 for hit, component, fraction in ITEM.findall(signature)]
        nonzero = [item for item in items if item[1] != 0]
        rows.append({
            "seed": int(match_event.group(1)),
            "entry": int(match_event.group(2)),
            "process_count": len(items),
            "nonidentity_process_count": len(nonzero),
            "g1_count": sum(item[1] == 1 for item in items),
            "g2_count": sum(item[1] == 2 for item in items),
            "g3_count": sum(item[1] == 3 for item in items),
            "g4_count": sum(item[1] == 4 for item in items),
            "cumulative_retained_fraction": math.prod(item[2] for item in items),
            "nonidentity_hits": ";".join(str(item[0]) for item in nonzero),
            "signature": signature,
        })
    rows.sort(key=lambda row: (row["seed"], row["entry"]))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(args.output.read_text(), end="")


if __name__ == "__main__":
    main()
