"""Validate persisted ownership/selection/override consistency; no fitting or writes."""
import collections
import math
import sys

import awkward as ak
import uproot


def check(path):
    with uproot.open(path, handler=uproot.source.file.MemmapSource) as f:
        rows = ak.to_list(f["breakpoint"].arrays(filter_name=["event_index", "hit_cell_id", "truth_ebrem_*",
            "truth_layer_association_status", "interval_selection*", "selected_breakpoint_interval",
            "truth_override_status", "truth_override_interval", "truth_override_retained_fraction",
            "truth_override_ebrem_loss"], library="ak"))
    counts = collections.Counter()
    ownership = collections.Counter()
    statuses = collections.Counter()
    events_without_owner = set()
    events_checked = set()
    for row in rows:
        counts["rows"] += 1
        if row["truth_layer_association_status"] != 1:
            counts["association_not_successful"] += 1
            continue
        event = row["event_index"]
        events_checked.add(event)
        steps = row["truth_ebrem_step"]
        n = len(steps)
        for suffix in ("owner_status", "breakpoint_status", "owner_system", "owner_layer",
                       "breakpoint_hit", "owner_key", "material_path", "x", "y", "z",
                       "momentum_before", "momentum_loss"):
            assert len(row["truth_ebrem_"+suffix]) == n, (event, suffix)
        offsets = row["truth_ebrem_candidate_layer_offset"]
        assert len(offsets) == n+1 and offsets[0] == 0
        assert offsets[-1] == len(row["truth_ebrem_candidate_layer"])
        assert all(a <= b for a, b in zip(offsets, offsets[1:]))
        owners = row["truth_ebrem_owner_status"]
        eligibility = row["truth_ebrem_breakpoint_status"]
        assert row["truth_ebrem_no_unique_owner_count"] == sum(o not in (1, 2) for o in owners)
        assert row["truth_ebrem_ineligible_count"] == sum(s != 1 for s in eligibility)
        ownership.update(owners); statuses.update(eligibility)
        if row["truth_ebrem_no_unique_owner_count"]:
            events_without_owner.add(event)
        groups = {}
        for i, (o, s, hit, loss) in enumerate(zip(owners, eligibility,
                row["truth_ebrem_breakpoint_hit"], row["truth_ebrem_momentum_loss"])):
            if hit >= 0:
                cell = row["hit_cell_id"][hit]
                assert row["truth_ebrem_owner_system"][i] == (cell & 31), (event, steps[i], "system")
                if row["truth_ebrem_owner_layer"][i] >= 0:
                    assert row["truth_ebrem_owner_layer"][i] == ((cell >> 7) & 511), (event, steps[i], "layer")
            if s != 1:
                continue
            assert o in (1, 2) and hit >= 0
            group = groups.setdefault(hit, [0., row["truth_ebrem_momentum_before"][i]])
            group[0] += loss
        if row["interval_selection_mode"] == "Truth":
            positive = {i: v for i, v in groups.items() if v[0] > 0}
            expected = [max(sorted(positive), key=lambda i: positive[i][0])] if positive else []
            assert row["selected_breakpoint_interval"] == expected, event
            assert row["interval_selection_status"] == (2 if expected else 3 if n == 0 else 4)
            if expected:
                loss, before = groups[expected[0]]
                assert math.isclose(row["interval_selection_truth_ebrem_loss"][0], loss)
                assert math.isclose(row["interval_selection_truth_retained_fraction"][0], 1.-loss/before)
        if row["truth_override_status"] == 1:
            for i, z, loss in zip(row["truth_override_interval"], row["truth_override_retained_fraction"],
                                  row["truth_override_ebrem_loss"]):
                summed, before = groups.get(i, [0., 1.])
                assert math.isclose(loss, summed) and math.isclose(z, 1.-summed/before)
        counts["checked_rows"] += 1
    print(path, dict(counts), "owner statuses", dict(ownership), "eligibility", dict(statuses),
          "events with no unique owner", len(events_without_owner), "/", len(events_checked))
    assert counts["checked_rows"], "No successfully associated rows checked"


if __name__ == "__main__":
    for filename in sys.argv[1:]:
        check(filename)
