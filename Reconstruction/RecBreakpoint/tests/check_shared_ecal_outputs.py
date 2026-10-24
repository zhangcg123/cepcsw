"""Compare focused old/new tuples; --free-only isolates the unchanged optimizer."""
import argparse
import math

import awkward as ak
import uproot


def rows(path):
    with uproot.open(path, handler=uproot.source.file.MemmapSource) as root:
        return ak.to_list(root["breakpoint"].arrays(library="ak"))


def equal(left, right):
    if isinstance(left, list):
        return isinstance(right, list) and len(left) == len(right) and all(
            equal(a, b) for a, b in zip(left, right))
    if isinstance(left, float) and isinstance(right, float):
        if math.isnan(left) and math.isnan(right):
            return True
    return left == right


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before")
    parser.add_argument("after")
    parser.add_argument("--free-only", action="store_true")
    parser.add_argument("--retired-beam", action="store_true", help="Allow only the explicitly removed beam-family fields")
    args = parser.parse_args()
    before, after = rows(args.before), rows(args.after)
    assert len(before) == len(after) and before, "Missing/mismatched track rows"
    checked = 0
    for old, new in zip(before, after):
        identity = (old["event_index"], old["input_track_index"])
        assert identity == (new["event_index"], new["input_track_index"])
        for key, value in old.items():
            if args.retired_beam and (key.startswith("beam_guided_free_loss_") or
                    key in {"free_loss_beam_nll2", "free_loss_beam_quadratic", "free_loss_beam_logdet",
                            "free_loss_beam_residual_mm", "free_loss_beam_innovation_variance_mm2",
                            "free_loss_beam_track_variance_mm2", "free_loss_beam_spot_variance_mm2",
                            "free_loss_trial_beam_nll2"}):
                assert key not in new, (identity, "retired field still present", key)
                continue
            if args.free_only and not key.startswith("free_loss_"):
                continue
            assert key in new and equal(value, new[key]), (identity, key)
            checked += 1
        if not args.free_only:
            assert new["ordinary_charged_ecal_measurement_status"] == 0
            assert new["ordinary_neutral_ecal_measurement_status"] == 0
            assert not new["ordinary_ecal_measurement_error"]
        if new["ordinary_likelihood_status"] == 1:
            expected = (new["ordinary_likelihood_quadratic"] + new["ordinary_likelihood_logdet"]
                        + new["ordinary_likelihood_measurement_dimensions"] * math.log(2 * math.pi))
            assert math.isclose(expected, new["ordinary_likelihood_nll2"], abs_tol=1e-8)
    print(f"PASS: {len(after)} rows, {checked} exact NaN-aware field comparisons")


if __name__ == "__main__":
    main()
