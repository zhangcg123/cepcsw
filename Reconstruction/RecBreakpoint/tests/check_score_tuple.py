"""Integration gate for default-on RTS score serialization.

After setup.sh: python3 check_score_tuple.py AFTER.root [--before BEFORE.root]
The reference check compares every pre-existing branch, not just endpoints.
"""
import argparse
import json
import math

import numpy as np
import ROOT

PREFIXES = ('', 'free_loss_', 'truth_override_')
LIKELIHOOD_PREFIXES = ('ordinary_likelihood_', 'free_loss_likelihood_',
                       'truth_override_likelihood_')
TERMS = ('smoothed_measurement_chi2', 'smoothed_process_chi2',
         'smoothed_native_measurement_chi2', 'smoothed_seed_chi2')


def read_rows(path):
    source = ROOT.TFile.Open(str(path))
    assert source and not source.IsZombie(), path
    tree = source.Get('breakpoint')
    assert tree, path
    branches = [(b.GetName(), b.GetClassName()) for b in tree.GetListOfBranches()]
    rows = []
    for entry in tree:
        row = {}
        for name, typename in branches:
            value = getattr(entry, name)
            if 'vector<' in typename:
                row[name] = [str(v) for v in value] if 'string' in typename else list(value)
            elif 'string' in typename:
                row[name] = str(value)
            else:
                row[name] = value
        rows.append(row)
    source.Close()
    return rows


def compare(a, b, name):
    if isinstance(a, str) or (isinstance(a, list) and a and isinstance(a[0], str)):
        assert a == b, name
        return True
    av, bv = np.asarray(a), np.asarray(b)
    assert av.shape == bv.shape, name
    assert np.allclose(av, bv, rtol=1e-10, atol=1e-9, equal_nan=True), name
    return bool(np.array_equal(av, bv, equal_nan=True))


def check_rows(rows):
    counts = dict(valid_scores=0, copies=0, unavailable=0)
    for row in rows:
        for prefix in LIKELIHOOD_PREFIXES:
            status = row[prefix + 'status']
            if status == 1:
                nll2 = row[prefix + 'nll2']
                quadratic = row[prefix + 'quadratic']
                logdet = row[prefix + 'logdet']
                dimensions = row[prefix + 'measurement_dimensions']
                assert dimensions > 0
                assert math.isclose(nll2, quadratic + logdet + dimensions * math.log(2 * math.pi),
                                    rel_tol=1e-11, abs_tol=1e-8)
                score_prefix = {'ordinary_likelihood_': '',
                                'free_loss_likelihood_': 'free_loss_',
                                'truth_override_likelihood_': 'truth_override_'}[prefix]
                assert math.isclose(quadratic, row[score_prefix + 'smoothed_chi2'],
                                    rel_tol=1e-9, abs_tol=1e-7)
            else:
                assert status in (0, -1)
                assert math.isnan(row[prefix + 'nll2'])
                if status == -1:
                    assert row[prefix + 'error']
        if row['free_loss_result_status'] == 1:
            for term in ('status', 'nll2', 'quadratic', 'logdet',
                         'measurement_dimensions', 'latent_dimensions', 'error'):
                assert compare(row['ordinary_likelihood_' + term],
                               row['free_loss_likelihood_' + term], term)
        if row['free_loss_result_status'] == 2:
            assert row['free_loss_likelihood_status'] == 1
            for term in ('nll2', 'quadratic', 'logdet'):
                assert math.isclose(row['free_loss_likelihood_' + term],
                                    row['free_loss_' + term], rel_tol=1e-12, abs_tol=1e-8)
        if row['truth_override_result_status'] == 1:
            for term in ('status', 'nll2', 'quadratic', 'logdet',
                         'measurement_dimensions', 'latent_dimensions', 'error'):
                assert compare(row['ordinary_likelihood_' + term],
                               row['truth_override_likelihood_' + term], term)
        for prefix in PREFIXES:
            for term in TERMS:
                assert prefix + term in row, prefix + term
            status = row[prefix + 'smoothed_chi2_status']
            measurement = np.asarray(row[prefix + TERMS[0]])
            process = np.asarray(row[prefix + TERMS[1]])
            native = np.asarray(row[prefix + TERMS[2]])
            seed = row[prefix + TERMS[3]]
            if prefix and row[prefix + 'result_status'] == 1:
                for term in TERMS:
                    assert compare(row[term], row[prefix + term], prefix + term), prefix + term
                counts['copies'] += 1
            if status == 1:
                n = row['hit_count']
                assert n > 0 and len(measurement) == len(process) == len(native) == n
                local = measurement + process
                local[0] += seed
                np.testing.assert_allclose(local, row[prefix + 'smoothed_local_chi2'], rtol=1e-12, atol=1e-10)
                np.testing.assert_allclose(local.sum(), row[prefix + 'smoothed_chi2'], rtol=1e-12, atol=1e-9)
                counts['valid_scores'] += 1
            elif status == 0:
                assert len(measurement) == len(process) == len(native) == 0
                assert math.isnan(seed)
                counts['unavailable'] += 1
            else:
                assert status == -1
                assert math.isnan(row[prefix + 'smoothed_chi2'])
                assert row[prefix + 'smoothed_chi2_error']
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('after')
    parser.add_argument('--before')
    args = parser.parse_args()
    after = read_rows(args.after)
    assert after, 'No attempted tracks in tuple'
    result = dict(rows=len(after), **check_rows(after))
    if args.before:
        before = read_rows(args.before)
        assert len(before) == len(after)
        exact = compared = 0
        for old, new in zip(before, after):
            for name, value in old.items():
                exact += compare(value, new[name], name)
                compared += 1
        result.update(old_fields_compared=compared, old_fields_exact=exact)
    print(json.dumps(result, sort_keys=True))


if __name__ == '__main__':
    main()
