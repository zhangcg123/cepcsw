#!/usr/bin/env python3
"""Check same-code recorder-off/on Higgs tuples and all non-material fields."""
import argparse
import json
import math

import ROOT


def value(tree, name):
    result = getattr(tree, name)
    return result if isinstance(result, (int, float, bool, str)) else list(result)


def equal(left, right):
    if isinstance(left, list):
        return len(left) == len(right) and all(equal(a, b) for a, b in zip(left, right))
    return left == right or (
        isinstance(left, float) and isinstance(right, float)
        and math.isnan(left) and math.isnan(right))


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
    fit_fields = sorted(name for name in fields if not name.startswith("truth_material_"))
    interval_fields = fields - set(fit_fields) - {
        "truth_material_track_status", "truth_material_scope_status",
        "truth_material_scope_valid", "truth_material_interval_n"}
    report = {"nonmaterial_fields": len(fit_fields), "events": []}
    for event in range(off.GetEntries()):
        off.GetEntry(event)
        on.GetEntry(event)
        for field in fit_fields:
            assert equal(value(off, field), value(on, field)), (event, field)
        assert not any(off.truth_material_track_status)
        assert off.truth_material_scope_status == 0
        assert off.truth_material_scope_valid == 0 and off.truth_material_interval_n == 0
        assert all(not value(off, field) for field in interval_fields)
        statuses = list(on.truth_material_track_status)
        if on.production_electron_selection_status == -5:
            assert statuses == [-4] and on.truth_material_interval_n == 0
            continue
        assert on.production_electron_selection_status in (1, 2), event
        selected = {index for index in on.production_electron_track_indices if index >= 0}
        assert all((status != 2) == (index in selected)
                   for index, status in enumerate(statuses)), (event, statuses)
        assert all(index < len(statuses) for index in selected)
        assert on.truth_bh_scope_status == 0 and on.truth_bh_scope_valid == 0
        outputs = list(on.gsf_input_track_indices)
        recorded = set(on.truth_material_input_track_index)
        assert recorded == {index for index, status in enumerate(statuses) if status == 1}
        assert recorded <= set(outputs)
        for original, output in zip(on.truth_material_input_track_index,
                                    on.truth_material_output_track_index):
            assert outputs[output] == original
        assert all(len(value(on, field)) == on.truth_material_interval_n
                   for field in interval_fields)
        report["events"].append({"event": event, "published": outputs,
                                 "statuses": {index: statuses[index] for index in sorted(selected)},
                                 "intervals": int(on.truth_material_interval_n)})
    assert report["events"] and any(row["intervals"] > 0 for row in report["events"])
    print(json.dumps(report, indent=2))
    for source in files:
        source.Close()


if __name__ == "__main__":
    main()
