"""Check the focused charged-routing runs against frozen pre-change outputs."""
import argparse
from pathlib import Path
import numpy as np
import uproot


def rows(path):
    tree = uproot.open(path, handler=uproot.source.file.MemmapSource)['breakpoint']
    return {(r['event_index'], r['input_track_index']): r for r in tree.arrays().to_list()}


def equal(a, b, names):
    for name in names:
        np.testing.assert_equal(a[name], b[name], err_msg=name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('old_electron')
    parser.add_argument('old_sum')
    args = parser.parse_args()
    runs = {name: rows(args.directory / (name + '.root')) for name in
            ('auto', 'electron', 'sum10', 'neutral_off', 'empty', 'empty_no_beam', 'off')}
    reference = rows(args.old_electron)
    old_sum = rows(args.old_sum)
    unchanged = ('kf_', 'rts_', 'free_loss_rts_', 'truth_override_', 'diffuse_')
    for key, old in reference.items():
        new = runs['electron'][key]
        names = old.keys() if old['selected_breakpoint_interval'] else (
            k for k in old if k.startswith(unchanged))
        equal(old, new, names)
    assert len(runs['sum10']) == len(old_sum) == 10
    for key, old in old_sum.items():
        new = runs['sum10'][key]
        equal(old, new, (k for k in old if k.startswith(
            unchanged + ('charged_ecal_', 'free_loss_charged_ecal_'))))
        assert new['charged_ecal_effective_energy_mode'] == 'ElectronPhoton'
        for method in ('ordinary', 'free_loss'):
            assert new[f'charged_ecal_{method}_constraint_status'] == 2
            assert new[f'neutral_ecal_{method}_constraint_mask'] == 2
            assert new[f'neutral_ecal_{method}_constraint_status'] == 1
            assert new[f'neutral_ecal_{method}_rts_pt'] == new[f'charged_ecal_{method}_rts_pt']
    no_breakpoint = resolved = unresolved = 0
    for key, row in runs['auto'].items():
        has_breakpoint = bool(row['selected_breakpoint_interval'])
        has_neutral = bool(row['collected_neutral_ecal_cluster_indices'])
        expected = 'ElectronPhoton' if has_breakpoint and not has_neutral else 'ElectronOnly'
        assert row['charged_ecal_effective_energy_mode'] == expected
        if expected == 'ElectronOnly':
            equal(row, runs['electron'][key], (k for k in row if k != 'charged_ecal_energy_mode'))
        if not has_breakpoint:
            if row['free_loss_charged_ecal_measurement_status'] == 2:
                no_breakpoint += 1
                assert row['charged_ecal_ordinary_constraint_status'] == 2
                assert row['charged_ecal_ordinary_constraint_mask'] == 3
                assert row['charged_ecal_ordinary_likelihood_status'] == 1
                assert row['charged_ecal_ordinary_smoothed_chi2_status'] == 1
                assert row['charged_ecal_ordinary_rts_pt'] == row['charged_ecal_free_loss_rts_pt']
        elif has_neutral:
            resolved += 1
            assert row['neutral_ecal_ordinary_constraint_mask'] == 7
            assert row['neutral_ecal_free_loss_constraint_mask'] == 7
        else:
            unresolved += 1
        equal(row, runs['off'][key], (k for k in row if k.startswith(unchanged)))
    for key, row in runs['neutral_off'].items():
        equal(row, runs['auto'][key], (k for k in row if k.startswith('charged_ecal_')))
    for name, mask in (('empty', 3), ('empty_no_beam', 2)):
        for row in runs[name].values():
            assert row['selected_breakpoint_interval'] == []
            assert row['charged_ecal_effective_energy_mode'] == 'ElectronOnly'
            for method in ('ordinary', 'free_loss'):
                prefix = f'charged_ecal_{method}_'
                assert row[prefix+'constraint_status'] == 2
                assert row[prefix+'constraint_mask'] == mask
                assert row[prefix+'smoothed_chi2_status'] == 1
            assert row['charged_ecal_ordinary_rts_pt'] == row['charged_ecal_free_loss_rts_pt']
            assert row['charged_ecal_ordinary_likelihood_status'] == 1
            base = 'beam_spot_ordinary_' if mask == 3 else ''
            likelihood = base + 'likelihood_' if base else 'ordinary_likelihood_'
            assert row['charged_ecal_ordinary_likelihood_measurement_dimensions'] == \
                row[likelihood+'measurement_dimensions'] + 1
            if base:
                np.testing.assert_equal(row['charged_ecal_ordinary_forward_filtered_parameters'],
                                        row[base+'forward_filtered_parameters'])
            assert np.isfinite(row['charged_ecal_ordinary_smoothed_chi2'])
            assert row['charged_ecal_ordinary_smoothed_covariance'][-13] < \
                row['charged_ecal_ordinary_forward_filtered_covariance'][-13]
    assert no_breakpoint and resolved and unresolved
    print(f'PASS: {len(runs["auto"])} rows: {no_breakpoint} no-breakpoint charged updates, '
          f'{resolved} resolved and {unresolved} unresolved breakpoint rows. '
          'Ten prior sum endpoints and prior electron-only breakpoint rows reproduce exactly; '
          'unconstrained endpoints unchanged; neutral-OFF routing, empty-list beam ON/OFF, '
          '5D measurement count, covariance contraction and free copies pass.')


if __name__ == '__main__':
    main()
