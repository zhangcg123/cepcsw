#!/usr/bin/env python3
"""Check every flat per-track endpoint against the same run's GSF EDM states."""
import argparse
import math

import uproot

from check_pre_ecal_tuple import equal


COLLECTIONS = {
    'gsf': 'GSFTracks',
    'bestbranch_gsf': 'GSFTracksBestBranch',
    'weighted_gsf': 'GSFTracksWeightedMean',
    'fullmixture_gsf': 'GSFTracksFullMixtureMode',
    'ecal_gsf': 'GSFTracksEcalConstrained',
}
PARAMETERS = {'d0': 'D0', 'phi': 'phi', 'omega': 'omega',
              'z0': 'Z0', 'tanl': 'tanLambda'}
OPEN = dict(handler=uproot.source.file.MemmapSource,
            decompression_executor=uproot.TrivialExecutor(),
            interpretation_executor=uproot.TrivialExecutor())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('edm')
    parser.add_argument('tuple')
    parser.add_argument('--baseline')
    parser.add_argument('--bfield', type=float, default=3.0)
    args = parser.parse_args()
    with uproot.open(args.edm, **OPEN) as source, uproot.open(args.tuple, **OPEN) as output:
        edm, tree = source['events'], output['gsf_tuple']
        assert edm.num_entries == tree.num_entries
        maps = edm['GSFTrackInputIndices'].array().to_list()
        flat_maps = tree['gsf_input_track_indices'].array().to_list()
        assert maps == flat_maps
        total = 0
        multiplicities = set()
        for prefix, collection in COLLECTIONS.items():
            fields = {key: tree[prefix+'_track_'+key].array().to_list()
                      for key in ['input_index', 'valid', *PARAMETERS, 'pT']}
            if collection not in edm:
                assert all(not row for values in fields.values() for row in values), prefix
                continue
            begin = edm[collection+'.trackStates_begin'].array().to_list()
            end = edm[collection+'.trackStates_end'].array().to_list()
            states = {key: edm['_'+collection+'_trackStates.'+key].array().to_list()
                      for key in ['location', *PARAMETERS.values()]}
            for event, inputs in enumerate(maps):
                assert fields['input_index'][event] == inputs, (prefix, event)
                assert len(begin[event]) == len(inputs)
                assert all(len(values[event]) == len(inputs) for values in fields.values())
                multiplicities.add(len(inputs))
                for index in range(len(inputs)):
                    ip = [i for i in range(begin[event][index], end[event][index])
                          if states['location'][event][i] == 1]  # edm4hep AtIP
                    assert len(ip) <= 1
                    expected = {key: states[value][event][ip[0]] if ip else math.nan
                                for key, value in PARAMETERS.items()}
                    for key, value in expected.items():
                        assert equal(fields[key][event][index], value), (prefix, event, index, key)
                    omega = expected['omega']
                    pt = (abs(args.bfield*2.99792458e-4/omega)
                          if math.isfinite(omega) and abs(omega) > 1e-15 else math.nan)
                    valid = all(math.isfinite(v) for v in expected.values()) and math.isfinite(pt) and pt > 0
                    assert fields['valid'][event][index] == int(valid)
                    assert equal(fields['pT'][event][index], pt if valid else math.nan)
                    if index == 0 and valid:
                        for key in [*PARAMETERS, 'pT']:
                            assert equal(tree[prefix+'_'+key].array(entry_start=event, entry_stop=event+1)[0],
                                         fields[key][event][index]), (prefix, event, key)
                    total += 1
        old_fields = 0
        if args.baseline:
            with uproot.open(args.baseline, **OPEN) as baseline:
                old = baseline['gsf_tuple']
                assert old.num_entries == tree.num_entries
                for key in old.keys():
                    assert equal(old[key].array().to_list(), tree[key].array().to_list()), key
                    old_fields += 1
        print(f'PASS: {tree.num_entries} events, {total} endpoint states, '
              f'published multiplicities={sorted(multiplicities)}; '
              f'{old_fields} existing fields unchanged')


if __name__ == '__main__':
    main()
