#!/usr/bin/env python3
"""Build primary-electron material transitions from G4 step tuples.

The input is produced by GsfMaterialStepRecorderAnaElemTool.  A transition uses
outgoing-current ownership: it starts at the Geant4 entry into one sensitive
element and ends immediately before entry into the next sensitive element on
the same primary track.  All Geant4 steps in that half-open interval are
aggregated exactly once.

The resulting g4_t_over_x0 is a Geant4 geometry truth diagnostic.  It is not
the component-local reconstruction t/X0; matching that value by reconstruction
surface identifier is a separate, required step before fitting the GSF model.
"""

import argparse
import csv
import glob
import json
import math
import os
import sys

import ROOT


GEOM_BOUNDARY = 1  # G4StepStatus::fGeomBoundary
REQUIRED_BRANCHES = {
    "run_id", "event_id", "step_count", "track_id", "parent_id", "pdg",
    "track_step_number", "step_status_pre", "pre_sensitive",
    "pre_touchable_path", "pre_x", "pre_y", "pre_z", "pre_r", "pre_p",
    "post_p", "step_tX0", "step_length", "loss", "process_subtype",
}

FIELDS = [
    "source_file", "run_id", "event_id", "track_id", "pdg",
    "transition_index", "surface_from", "surface_to",
    "from_track_step", "to_track_step", "first_recorded_index",
    "last_recorded_index", "n_steps", "n_ebrem_steps",
    "from_x_mm", "from_y_mm", "from_z_mm", "from_r_mm",
    "to_x_mm", "to_y_mm", "to_z_mm", "to_r_mm",
    "p_before_GeV", "p_after_GeV", "z", "minus_log_z",
    "loss_GeV", "ebrem_step_loss_sum_GeV", "path_length_mm",
    "g4_t_over_x0", "material_t_over_x0", "process_step_counts",
    "reco_t_over_x0",
]


def expand_inputs(patterns):
    paths = []
    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if matches:
            paths.extend(matches)
        elif os.path.isfile(pattern):
            paths.append(pattern)
        else:
            raise FileNotFoundError("no input matches: %s" % pattern)
    return list(dict.fromkeys(os.path.abspath(path) for path in paths))


def check_tree(tree, source):
    branches = {branch.GetName() for branch in tree.GetListOfBranches()}
    missing = sorted(REQUIRED_BRANCHES - branches)
    if missing:
        raise RuntimeError(
            "%s lacks transition-producer branches: %s" %
            (source, ", ".join(missing)))


def primary_tracks(tree):
    tracks = {}
    for index in range(int(tree.step_count)):
        if int(tree.parent_id[index]) != 0 or abs(int(tree.pdg[index])) != 11:
            continue
        tracks.setdefault(int(tree.track_id[index]), []).append(index)
    for indices in tracks.values():
        indices.sort(key=lambda i: (int(tree.track_step_number[i]), i))
    return tracks


def sensitive_entry_anchors(tree, indices):
    anchors = []
    last_key = None
    for index in indices:
        if not int(tree.pre_sensitive[index]):
            continue
        if int(tree.step_status_pre[index]) != GEOM_BOUNDARY:
            continue
        path = str(tree.pre_touchable_path[index])
        # CEPC's 5 mm TPC pad row is implemented as adjacent 2.5 mm lower and
        # upper sensitive volumes.  CompleteTracks contains one measurement
        # per pair.  For an IP-originating outward track, the lower-volume
        # entry is therefore the reconstruction-surface anchor.
        if "/TPC_upperlayer_log_" in path:
            continue
        key = (path, int(tree.track_step_number[index]))
        if key == last_key:
            continue
        anchors.append(index)
        last_key = key
    return anchors


def transition_row(tree, source, track_id, transition_index, indices,
                   start_index, stop_index):
    position = {recorded: order for order, recorded in enumerate(indices)}
    begin = position[start_index]
    end = position[stop_index]
    owned = indices[begin:end]
    if not owned:
        return None

    p_before = float(tree.pre_p[start_index])
    p_after = float(tree.pre_p[stop_index])
    z = p_after / p_before if p_before > 0.0 else float("nan")
    minus_log_z = -math.log(z) if z > 0.0 else float("nan")
    material_tx0 = {}
    process_counts = {}
    for index in owned:
        material = str(tree.material[index]) or "<none>"
        process = str(tree.process[index]) or "<none>"
        material_tx0[material] = material_tx0.get(material, 0.0) + float(
            tree.step_tX0[index])
        process_counts[process] = process_counts.get(process, 0) + 1

    return {
        "source_file": source,
        "run_id": int(tree.run_id),
        "event_id": int(tree.event_id),
        "track_id": track_id,
        "pdg": int(tree.pdg[start_index]),
        "transition_index": transition_index,
        "surface_from": str(tree.pre_touchable_path[start_index]),
        "surface_to": str(tree.pre_touchable_path[stop_index]),
        "from_track_step": int(tree.track_step_number[start_index]),
        "to_track_step": int(tree.track_step_number[stop_index]),
        "first_recorded_index": owned[0],
        "last_recorded_index": owned[-1],
        "n_steps": len(owned),
        "n_ebrem_steps": sum(
            1 for i in owned if int(tree.process_subtype[i]) == 3),
        "from_x_mm": float(tree.pre_x[start_index]),
        "from_y_mm": float(tree.pre_y[start_index]),
        "from_z_mm": float(tree.pre_z[start_index]),
        "from_r_mm": float(tree.pre_r[start_index]),
        "to_x_mm": float(tree.pre_x[stop_index]),
        "to_y_mm": float(tree.pre_y[stop_index]),
        "to_z_mm": float(tree.pre_z[stop_index]),
        "to_r_mm": float(tree.pre_r[stop_index]),
        "p_before_GeV": p_before,
        "p_after_GeV": p_after,
        "z": z,
        "minus_log_z": minus_log_z,
        "loss_GeV": p_before - p_after,
        "ebrem_step_loss_sum_GeV": sum(
            float(tree.loss[i]) for i in owned
            if int(tree.process_subtype[i]) == 3),
        "path_length_mm": sum(float(tree.step_length[i]) for i in owned),
        "g4_t_over_x0": sum(float(tree.step_tX0[i]) for i in owned),
        "material_t_over_x0": "|".join(
            "%s:%.17g" % item for item in sorted(material_tx0.items())),
        "process_step_counts": "|".join(
            "%s:%d" % item for item in sorted(process_counts.items())),
        "reco_t_over_x0": "",
    }


def build(paths):
    rows = []
    audit = {
        "ownership": "sensitive-entry inclusive to next sensitive-entry exclusive",
        "input_files": len(paths),
        "events": 0,
        "primary_tracks": 0,
        "sensitive_anchors": 0,
        "transitions": 0,
        "events_without_two_anchors": 0,
        "nonfinite_z": 0,
        "z_above_one": 0,
    }

    for path in paths:
        root_file = ROOT.TFile.Open(path)
        if not root_file or root_file.IsZombie():
            raise RuntimeError("cannot open %s" % path)
        tree = root_file.Get("g4step_tuple")
        if not tree:
            raise RuntimeError("%s has no g4step_tuple" % path)
        check_tree(tree, path)

        source = os.path.basename(path)
        for entry in range(tree.GetEntries()):
            tree.GetEntry(entry)
            audit["events"] += 1
            event_has_transition = False
            for track_id, indices in primary_tracks(tree).items():
                audit["primary_tracks"] += 1
                anchors = sensitive_entry_anchors(tree, indices)
                audit["sensitive_anchors"] += len(anchors)
                for transition_index, (start, stop) in enumerate(
                        zip(anchors, anchors[1:])):
                    row = transition_row(
                        tree, source, track_id, transition_index,
                        indices, start, stop)
                    if row is None:
                        continue
                    rows.append(row)
                    event_has_transition = True
                    audit["transitions"] += 1
                    if not math.isfinite(row["z"]):
                        audit["nonfinite_z"] += 1
                    if row["z"] > 1.0 + 1e-6:
                        audit["z_above_one"] += 1
            if not event_has_transition:
                audit["events_without_two_anchors"] += 1
        root_file.Close()
    return rows, audit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", help="Input ROOT files or globs")
    parser.add_argument("--output", required=True, help="Output transition CSV")
    parser.add_argument("--audit", help="Audit JSON (default: OUTPUT.audit.json)")
    args = parser.parse_args()

    try:
        paths = expand_inputs(args.inputs)
        rows, audit = build(paths)
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        with open(args.output, "w", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=FIELDS)
            writer.writeheader()
            writer.writerows(rows)
        audit_path = args.audit or args.output + ".audit.json"
        with open(audit_path, "w") as stream:
            json.dump(audit, stream, indent=2, sort_keys=True)
            stream.write("\n")
    except Exception as error:
        print("error: %s" % error, file=sys.stderr)
        return 1

    print("wrote %d transitions from %d events to %s" %
          (audit["transitions"], audit["events"], args.output))
    print("audit: %s" % audit_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
