#!/usr/bin/env python3
"""Compare matched G4 and RecGsfTracking material transitions.

The G4 transition index and GSF hit index share outgoing-current ownership.
When several GSF components exist, their path t/X0 values are weight-averaged
for the material comparison; component-local rows remain in the source CSV.
"""

import argparse
import csv
import json
import math
import statistics


FIELDS = [
    "event_id", "transition_index", "surface_from", "surface_to",
    "g4_from_r_mm", "g4_to_r_mm", "reco_from_r_mm", "reco_to_r_mm",
    "n_g4_steps", "n_ebrem_steps", "g4_t_over_x0",
    "reco_t_over_x0", "g4_over_reco", "g4_minus_reco",
    "p_before_GeV", "p_after_GeV", "z", "minus_log_z",
]


def read_g4(path, event_id):
    with open(path, newline="") as stream:
        rows = [row for row in csv.DictReader(stream)
                if int(row["event_id"]) == event_id]
    return {int(row["transition_index"]): row for row in rows}


def read_reco(path, event_id, track_index, tx0_column):
    grouped = {}
    with open(path, newline="") as stream:
        for row in csv.DictReader(stream):
            if int(row["event_index"]) != event_id:
                continue
            if int(row["track_index"]) != track_index:
                continue
            grouped.setdefault(int(row["hit_index"]), []).append(row)

    result = {}
    for hit_index, rows in grouped.items():
        valid = [row for row in rows if int(row["valid"])]
        weight_sum = sum(float(row["component_weight"]) for row in valid)
        if weight_sum > 0.0:
            path_tx0 = sum(float(row["component_weight"]) *
                           float(row[tx0_column]) for row in valid)
            path_tx0 /= weight_sum
        else:
            path_tx0 = float("nan")
        result[hit_index] = (rows[0], path_tx0, len(rows))
    return result


def classify(row):
    source = row["surface_from"]
    target = row["surface_to"]
    if "TPC" in source and "TPC" in target:
        return "tpc_to_tpc"
    if "VXD" in source and "VXD" in target:
        return "vxd_to_vxd"
    return "inter_detector_or_support"


def compare(g4, reco, event_id):
    common = sorted(set(g4) & set(reco))
    rows = []
    ratios_by_class = {}
    for index in common:
        truth = g4[index]
        reconstruction, reco_tx0, _ = reco[index]
        g4_tx0 = float(truth["g4_t_over_x0"])
        ratio = g4_tx0 / reco_tx0 if reco_tx0 > 0.0 else float("nan")
        row = {
            "event_id": event_id,
            "transition_index": index,
            "surface_from": truth["surface_from"],
            "surface_to": truth["surface_to"],
            "g4_from_r_mm": truth["from_r_mm"],
            "g4_to_r_mm": truth["to_r_mm"],
            "reco_from_r_mm": reconstruction["from_r_mm"],
            "reco_to_r_mm": reconstruction["to_r_mm"],
            "n_g4_steps": truth["n_steps"],
            "n_ebrem_steps": truth["n_ebrem_steps"],
            "g4_t_over_x0": g4_tx0,
            "reco_t_over_x0": reco_tx0,
            "g4_over_reco": ratio,
            "g4_minus_reco": g4_tx0 - reco_tx0,
            "p_before_GeV": truth["p_before_GeV"],
            "p_after_GeV": truth["p_after_GeV"],
            "z": truth["z"],
            "minus_log_z": truth["minus_log_z"],
        }
        rows.append(row)
        if math.isfinite(ratio):
            ratios_by_class.setdefault(classify(truth), []).append(ratio)

    sum_g4 = sum(row["g4_t_over_x0"] for row in rows)
    sum_reco = sum(row["reco_t_over_x0"] for row in rows
                   if math.isfinite(row["reco_t_over_x0"]))
    missing_g4 = sorted(set(reco) - set(g4))
    missing_reco = sorted(set(g4) - set(reco))
    audit = {
        "event_id": event_id,
        "matched_transitions": len(rows),
        "g4_t_over_x0_sum": sum_g4,
        "reco_t_over_x0_sum": sum_reco,
        "g4_over_reco_sum_ratio": sum_g4 / sum_reco if sum_reco > 0 else None,
        "g4_indices_without_reco": missing_reco,
        "reco_indices_without_g4": missing_g4,
        "unprocessed_seed_transition": g4.get(0),
        "ebrem_matched_transitions": [
            row for row in rows if int(row["n_ebrem_steps"]) > 0],
        "median_g4_over_reco_by_class": {
            key: statistics.median(values)
            for key, values in sorted(ratios_by_class.items())
        },
    }
    return rows, audit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--g4", required=True, help="G4 transition CSV")
    parser.add_argument("--reco", required=True, help="GSF material CSV")
    parser.add_argument("--event", required=True, type=int)
    parser.add_argument("--track", type=int, default=0)
    parser.add_argument(
        "--reco-column", default="path_t_over_x0",
        choices=("path_t_over_x0", "interval_path_t_over_x0",
                 "geometry_path_t_over_x0"),
        help="Reconstruction t/X0 column to compare")
    parser.add_argument("--output", required=True, help="Matched comparison CSV")
    parser.add_argument("--audit", help="Audit JSON (default: OUTPUT.audit.json)")
    args = parser.parse_args()

    g4 = read_g4(args.g4, args.event)
    reco = read_reco(args.reco, args.event, args.track, args.reco_column)
    rows, audit = compare(g4, reco, args.event)
    audit["reco_t_over_x0_column"] = args.reco_column
    with open(args.output, "w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    audit_path = args.audit or args.output + ".audit.json"
    with open(audit_path, "w") as stream:
        json.dump(audit, stream, indent=2, sort_keys=True)
        stream.write("\n")

    print("matched %d transitions" % len(rows))
    print("sum t/X0: G4 %.9g reco %.9g ratio %.6g" % (
        audit["g4_t_over_x0_sum"], audit["reco_t_over_x0_sum"],
        audit["g4_over_reco_sum_ratio"]))
    print("audit: %s" % audit_path)


if __name__ == "__main__":
    main()
