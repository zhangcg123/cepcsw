"""Focused same-observation fixture: old runtime, legacy input, pre-PFO input, rec."""
import argparse
import awkward as ak
import numpy as np
import uproot


def compare(left, right, ignored=()):
    count = 0
    assert left.num_entries == right.num_entries
    for name in left.keys():
        if name in ignored:
            continue
        a, b = left[name].array(), right[name].array()
        np.testing.assert_array_equal(ak.to_numpy(ak.flatten(a, axis=None)),
                                      ak.to_numpy(ak.flatten(b, axis=None)), err_msg=name)
        if a.ndim > 1:
            np.testing.assert_array_equal(ak.to_numpy(ak.num(a, axis=1)),
                                          ak.to_numpy(ak.num(b, axis=1)), err_msg=name)
        count += 1
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('reference', 'legacy', 'pre', 'rec'):
        parser.add_argument(name)
    args = parser.parse_args()
    files = [uproot.open(path, handler=uproot.source.file.MemmapSource)
             for path in (args.reference, args.legacy, args.pre, args.rec)]
    reference, legacy, pre = (file['breakpoint'] for file in files[:3])
    old_count = compare(reference, legacy)
    ignored = ('charged_ecal_input_clusters', 'ecal_constraint_input',
               'ecal_constraint_input_pfos', 'ecal_constraint_input_clusters',
               'charged_ecal_source_cluster_indices', 'collected_neutral_ecal_cluster_indices')
    compare(legacy, pre, ignored)
    side_count = compare(files[1]['neutral_pfos'], files[2]['neutral_pfos'])
    for field, expected in (
            ('ecal_constraint_input', 'PreReclustering'),
            ('ecal_constraint_input_pfos', 'CyberPFOPreReclustering'),
            ('ecal_constraint_input_clusters', 'EcalClusterPreReclustering'),
            ('charged_ecal_input_clusters', 'EcalClusterPreReclustering')):
        assert set(pre[field].array().to_list()) == {expected}, field
    energy = files[3]['events']['EcalClusterPreReclustering/EcalClusterPreReclustering.energy'].array()
    records = pre.arrays().to_list()
    charged = neutral = 0
    for row in records:
        values = energy[row['event_index']]
        for indices, measured in (
                ('charged_ecal_source_cluster_indices', 'free_loss_charged_ecal_energy'),
                ('collected_neutral_ecal_cluster_indices', 'collected_neutral_ecal_energy')):
            selected = row[indices]
            if selected:
                assert all(0 <= i < len(values) for i in selected)
                assert row[measured] == sum(float(values[i]) for i in selected), measured
                if indices.startswith('charged'):
                    charged += 1
                else:
                    neutral += 1
    assert charged and neutral, 'Fixture must exercise both ECAL input paths'
    print(f'PASS: {len(records)} rows; {old_count} old fields reproduced exactly; '
          f'pre-PFO fixture has identical fit outputs and {side_count} unchanged final-PFO '
          f'diagnostic fields; {charged} charged/{neutral} neutral selections read snapshot energies.')


if __name__ == '__main__':
    main()
