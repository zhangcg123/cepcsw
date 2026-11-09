#!/usr/bin/env python3
"""Check automatic oracle scope and unchanged whole-track fallbacks in Higgs tuples."""
import argparse
import json

import ROOT

from check_truth_material_recording import equal, value


def track_rows(tree, family, original, fields):
    indices = list(getattr(tree, family + "input_track_index"))
    selected = [row for row, index in enumerate(indices) if index == original]
    result = {}
    for field in fields:
        if not field.startswith(family) or field.endswith("output_track_index"):
            continue
        entries = value(tree, field)
        if isinstance(entries, list):
            assert len(entries) == len(indices), field
            result[field] = [entries[row] for row in selected]
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("off")
    parser.add_argument("on")
    args = parser.parse_args()
    files = [ROOT.TFile.Open(path, "READ") for path in (args.off, args.on)]
    assert all(source and not source.IsZombie() for source in files)
    off, on = [source.Get("gsf_tuple") for source in files]
    assert off and on and off.GetEntries() == on.GetEntries()
    fields = {branch.GetName() for branch in off.GetListOfBranches()}
    assert fields == {branch.GetName() for branch in on.GetListOfBranches()}
    report = {"events": [], "exact_fallback_tracks": 0,
              "exact_fallback_event_fields": 0, "oracle_changed_population": False}
    for event in range(off.GetEntries()):
        off.GetEntry(event)
        on.GetEntry(event)
        assert not any(off.truth_bh_track_status)
        assert on.truth_material_interval_n == off.truth_material_interval_n == 0
        assert not any(on.truth_material_track_status)
        statuses = list(on.truth_bh_track_status)
        if on.production_electron_selection_status == -5:
            assert statuses == [-4]
            continue
        assert on.production_electron_selection_status in (1, 2), event
        selected = {index for index in on.production_electron_track_indices if index >= 0}
        assert all(index < len(statuses) for index in selected)
        assert all((status != 2) == (index in selected)
                   for index, status in enumerate(statuses))
        if selected and all(statuses[index] < 0 for index in selected):
            for field in fields:
                if not field.startswith("truth_bh_"):
                    assert equal(value(off, field), value(on, field)), (event, field)
                    report["exact_fallback_event_fields"] += 1
        for original in selected:
            if statuses[original] < 0:
                assert original in on.gsf_input_track_indices
                assert original in off.gsf_input_track_indices
                for family in ("final_mixture_component_", "lineage_node_", "lineage_edge_"):
                    left = track_rows(off, family, original, fields)
                    right = track_rows(on, family, original, fields)
                    assert left.keys() == right.keys()
                    for field in left:
                        assert equal(left[field], right[field]), (event, original, field)
                report["exact_fallback_tracks"] += 1
            elif statuses[original] == 1:
                left = track_rows(off, "final_mixture_component_", original, fields)
                right = track_rows(on, "final_mixture_component_", original, fields)
                report["oracle_changed_population"] |= any(
                    not equal(left[field], right[field]) for field in left)
        report["events"].append({"event": event,
                                 "statuses": {index: statuses[index] for index in sorted(selected)},
                                 "published_off": list(off.gsf_input_track_indices),
                                 "published_on": list(on.gsf_input_track_indices)})
    assert report["events"] and report["oracle_changed_population"]
    assert report["exact_fallback_tracks"] > 0
    print(json.dumps(report, indent=2))
    for source in files:
        source.Close()


if __name__ == "__main__":
    main()
