"""Check native ECAL (or --beam boundary) F/Q closure in BP_VERBOSE=1 logs."""
import re
import sys

import numpy as np


def check(block):
    size = len(block["source_mean"])
    assert size in (5, 6)
    source = block["source_covariance"].reshape(size, size)
    predicted = block["predicted_covariance"].reshape(size, size)
    filtered = block["filtered_covariance"].reshape(size, size)
    transport = block["transport"].reshape(size, size)
    noise = block["noise"].reshape(size, size)
    scale = np.sqrt(np.diag(predicted))
    assert np.isfinite(scale).all() and (scale > 0).all()
    units = np.outer(scale, scale)
    closure = (predicted - transport @ source @ transport.T - noise) / units
    assert np.max(np.abs(closure)) < 1e-10, (block["event"], "transport closure")
    for name, covariance in (("predicted", predicted), ("filtered", filtered), ("noise", noise)):
        normalized = covariance / units
        assert np.max(np.abs(normalized - normalized.T)) < 1e-10
        assert np.linalg.eigvalsh(normalized).min() >= -1e-12, (block["event"], name)
    if size == 6:
        assert np.array_equal(transport[5], np.eye(6)[5])
        assert np.array_equal(transport[:, 5], np.eye(6)[:, 5])
        assert np.count_nonzero(noise[5]) == np.count_nonzero(noise[:, 5]) == 0
        assert block["source_mean"][5] == block["predicted_mean"][5]
    assert block["predicted_mean"][2] * block["filtered_mean"][2] > 0
    return float(np.max(np.abs(closure)))


def main(paths):
    beam = paths and paths[0] == '--beam'
    if beam:
        paths = paths[1:]
    header = 'BeamSpotKF' if beam else 'ChargedEnergyKF'
    prefix = 'BeamBoundaryKF' if beam else 'ChargedEnergyKF'
    count, worst = 0, 0.0
    for path in paths:
        block = None
        with open(path) as stream:
            for line in stream:
                start = re.search(header + r" event=(\d+) track=(\d+)", line)
                if start:
                    assert block is None, "Incomplete previous native transport dump"
                    block = {"event": tuple(map(int, start.groups()))}
                item = re.search(prefix + r" (\w+)=\[([^]]*)\]", line)
                if item:
                    assert block is not None
                    block[item[1]] = np.fromstring(item[2], sep=" ")
                    if item[1] == "noise":
                        worst = max(worst, check(block))
                        count += 1
                        block = None
        assert block is None, "Incomplete final native transport dump"
    assert count, "No requested native transport dumps found"
    print(f"PASS: {count} native {prefix} edges; max normalized closure error {worst:.3g}")


if __name__ == "__main__":
    main(sys.argv[1:])
