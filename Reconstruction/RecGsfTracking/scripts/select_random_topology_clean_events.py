#!/usr/bin/env python3
"""Select a reproducible uniform random sample from topology-clean outcomes."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import random


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--outcomes", type=Path, required=True)
    parser.add_argument("--category", required=True)
    parser.add_argument("--count", type=int, required=True)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    with args.outcomes.open(newline="") as stream:
        population = [row for row in csv.DictReader(stream)
                      if row["category"] == args.category
                      and not int(row["excluded_secondary_topology"])]
    if args.count > len(population):
        raise ValueError(f"requested {args.count} from population {len(population)}")
    sample = random.Random(args.seed).sample(population, args.count)
    sample.sort(key=lambda row: (int(row["seed"]), int(row["entry"])))
    fields = ["seed", "entry", "category", "outcome", "owned_loss_pct",
              "lcio_residual_pct", "gsf_residual_pct", "sampling_seed",
              "sampling_population"]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for row in sample:
            writer.writerow({key: row[key] for key in fields[:7]} | {
                "sampling_seed": args.seed,
                "sampling_population": len(population),
            })
    print(f"selected {len(sample)} of {len(population)} {args.category} events "
          f"with seed {args.seed} -> {args.output}")


if __name__ == "__main__":
    main()
