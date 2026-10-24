"""Check OFF NaNs, independent ON families, and unchanged unconstrained results."""
import argparse
import math
from check_shared_ecal_outputs import rows, equal

KINDS = ("beam_spot", "charged_ecal", "neutral_ecal")
BASE_PREFIXES = ("rts_", "ordinary_likelihood_", "free_loss_rts_",
                 "free_loss_likelihood_", "truth_override_", "diffuse_augmented_",
                 "backward_", "free_loss_backward_")


def index(path):
    result = {}
    for row in rows(path):
        key = row["event_index"], row["input_track_index"]
        assert key not in result, key
        result[key] = row
    assert result, "Empty test input"
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tuple")
    parser.add_argument("--enabled", default="", help="Comma-separated beam_spot,charged_ecal,neutral_ecal")
    parser.add_argument("--baseline", help="Same events with all switches OFF")
    parser.add_argument("--independent", help="Same events with other switches changed")
    args = parser.parse_args()
    enabled = set(filter(None, args.enabled.split(",")))
    assert enabled <= set(KINDS)
    data = index(args.tuple)
    compared = 0
    for key, row in data.items():
        for kind in KINDS:
            for method in ("ordinary", "free_loss"):
                prefix = f"{kind}_{method}_"
                assert not any(k.startswith(prefix + "backward_") for k in row)
                status = row[prefix + "constraint_status"]
                if kind not in enabled:
                    assert status == 0 and row[prefix + "rts_index"] == -1, (key, prefix)
                    for name in ("pt", "pt_error", "calo_pt", "calo_pt_error"):
                        assert math.isnan(row[prefix + "rts_" + name]), (key, prefix, name)
                    assert row[prefix + "rts_ip_parameters"] == []
                    assert row[prefix + "rts_ip_covariance"] == []
                else:
                    assert status in (1, 2, -1), (key, prefix, status)
                    assert row[prefix + "rts_index"] >= 0, (key, prefix)
                    assert math.isfinite(row[prefix + "rts_pt"]), (key, prefix)
                if row[prefix + "likelihood_status"] == 1:
                    expected = (row[prefix + "likelihood_quadratic"]
                                + row[prefix + "likelihood_logdet"]
                                + row[prefix + "likelihood_measurement_dimensions"] * math.log(2*math.pi))
                    assert math.isclose(expected, row[prefix + "likelihood_nll2"], abs_tol=1e-8)
    for path, prefixes in ((args.baseline, BASE_PREFIXES),
                           (args.independent, tuple(f"{kind}_" for kind in enabled))):
        if not path:
            continue
        other = index(path)
        common = data.keys() & other.keys()
        assert common, "No paired events"
        for key in common:
            for field, value in data[key].items():
                if field.startswith(prefixes):
                    assert field in other[key] and equal(value, other[key][field]), (key, field)
                    compared += 1
    print(f"PASS: {len(data)} tracks; OFF/ON contracts; {compared} exact paired field checks")


if __name__ == "__main__":
    main()
