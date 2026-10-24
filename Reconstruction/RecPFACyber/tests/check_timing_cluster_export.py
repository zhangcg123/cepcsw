"""Check passive export using full-chain, timing-only and original rec outputs."""
import argparse

import awkward as ak
import numpy as np
import uproot


def values(tree, field, entries=None):
    data = tree[field].array()
    if entries is not None:
        data = data[entries]
    return ak.to_numpy(ak.flatten(data, axis=None))


def check(full_path, upstream_path, reference_path, entries):
    options = dict(handler=uproot.source.file.MemmapSource)
    with uproot.open(full_path, **options) as full_file, \
            uproot.open(upstream_path, **options) as upstream_file, \
            uproot.open(reference_path, **options) as reference_file:
        full, upstream, reference = (f['events'] for f in
                                     (full_file, upstream_file, reference_file))
        assert full.num_entries == upstream.num_entries == len(entries)
        assert len(full.keys()) == len(set(full.keys())), 'Duplicate ROOT branches'
        timing = [k for k in full.keys() if k.startswith(
            ('EcalClusterTiming/', 'EcalClusterTimingHits/', '_EcalClusterTiming_hits/'))]
        assert timing, 'No timing clusters exported'
        for field in timing:
            assert ak.to_list(ak.num(full[field].array(), axis=1)) == \
                ak.to_list(ak.num(upstream[field].array(), axis=1)), field
            np.testing.assert_array_equal(values(full, field), values(upstream, field),
                                          err_msg=field)
        prefixes = ('CyberPFO/', 'CyberPFOPID/', 'EcalCluster/', 'EcalCore/',
                    'HcalCluster/', 'RecECALBarrel/', 'RecECALBarrelCore/',
                    'RecHCALBarrel/', '_CyberPFO_', '_CyberPFOPID_',
                    '_EcalCluster_', '_EcalCore_', '_HcalCluster_')
        final = [k for k in full.keys() if k.startswith(prefixes) and k in reference.keys()]
        assert final, 'No final PFA fields compared'
        for field in final:
            assert ak.to_list(ak.num(full[field].array(), axis=1)) == \
                ak.to_list(ak.num(reference[field].array()[entries], axis=1)), field
            np.testing.assert_array_equal(values(full, field), values(reference, field, entries),
                                          err_msg=field)
        for collection in ('EcalClusterTiming', 'EcalClusterTimingHits'):
            energy = values(full, collection + '/' + collection + '.energy')
            assert len(energy) and np.all(np.isfinite(energy)) and np.all(energy >= 0)
        print(f'PASS: {full.num_entries} events; {len(timing)} timing fields match the '
              f'upstream-only run; {len(final)} final PFA fields match the original rec.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('full')
    parser.add_argument('upstream')
    parser.add_argument('reference')
    parser.add_argument('--entries', required=True, help='Original zero-based entries, comma-separated')
    args = parser.parse_args()
    check(args.full, args.upstream, args.reference, list(map(int, args.entries.split(','))))
