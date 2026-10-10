"""Read-only schema/numerical gate; optionally prove truth-on/off isolation.

python check_interval_identification.py diagnostic.root [truth_off.root]
Uses uproot/awkward; no CEPC runtime needed. This is not physics validation.
"""
import sys
import awkward as ak
import numpy as np
import uproot


def read(path):
    executor = uproot.source.futures.TrivialExecutor()
    return uproot.open(path, handler=uproot.source.file.MultithreadedFileSource,
                       num_workers=1, decompression_executor=executor,
                       interpretation_executor=executor)["interval_identification"].arrays(library="ak")


def check(data):
    assert len(data) > 0
    summary = []
    for row in ak.to_list(data):
        n = row["hit_count"]
        assert len(row["hit_position"]) == n
        assert len(row["interval_upstream_hit"]) == max(0, n - 1)
        if "interval_chord_tx0" in row:
            for reverse in (False, True):
                suffix = "reverse_" if reverse else ""
                tx0 = row["interval_chord_" + suffix + "tx0"]
                status = row["interval_chord_material_" + suffix + "status"]
                covered = row["interval_chord_material_" + suffix + "covered_mm"]
                assert len(tx0) == len(status) == len(covered) == n - 1
                for value, flag, path, length in zip(tx0, status, covered, row["interval_chord_length_mm"]):
                    assert flag in (-1, 1, 2)
                    if flag > 0:
                        assert np.isfinite(value) and value >= 0
                        assert abs(path - length) <= max(1e-3, 1e-6 * length)
                    else:
                        assert np.isnan(value)
        for direction in ("forward_", "backward_"):
            assert len(row[direction + "update_status"]) == n
            assert any(x == 1 for x in row[direction + "diagnostic_valid"]), direction
            for i, valid in enumerate(row[direction + "diagnostic_valid"]):
                if valid != 1:
                    continue
                d = row[direction + "measurement_dimension"][i]
                assert len(row[direction + "predicted_parameters"][i]) == 5
                assert len(row[direction + "predicted_covariance"][i]) == 25
                r = np.asarray(row[direction + "innovation"][i])
                s = np.asarray(row[direction + "innovation_covariance"][i]).reshape(d, d)
                assert np.all(np.isfinite(r)) and np.all(np.linalg.eigvalsh(s) > 0)
                q = float(r @ np.linalg.solve(s, r))
                np.testing.assert_allclose(q, row[direction + "innovation_chi2"][i], rtol=1e-7, atol=1e-7)
        if row["truth_status"] > 0:
            assert len(row["truth_interval_status"]) == n - 1
            losses = np.zeros(n - 1)
            for i, loss in zip(row["truth_ebrem_interval"], row["truth_ebrem_momentum_loss"]):
                if i >= 0:
                    losses[i] += loss
            np.testing.assert_allclose(losses, row["truth_interval_ebrem_momentum_loss"], rtol=1e-6, atol=1e-7)
        summary.append((row["event_index"], row["input_track_index"], n,
                        row["forward_status"], row["backward_status"], row["truth_status"]))
    print("event, track, hits, forward, backward, truth:", summary, flush=True)


data = read(sys.argv[1])
check(data)
if len(sys.argv) > 2:
    other = read(sys.argv[2])
    assert len(data) == len(other)
    for name in data.fields:
        if name.startswith("truth_"):
            continue
        # Compare nested structures with NaNs equal, including string errors.
        left, right = ak.to_list(data[name]), ak.to_list(other[name])
        def equal(a, b):
            if isinstance(a, list):
                return len(a) == len(b) and all(equal(x, y) for x, y in zip(a, b))
            return a == b or (isinstance(a, float) and isinstance(b, float) and np.isnan(a) and np.isnan(b))
        assert equal(left, right), name
    print("All non-truth branches identical with truth recording disabled.", flush=True)
