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


def compare_snapshot(full, upstream, name):
    fields = [k for k in full.keys() if k.startswith(
        (name+'/', name+'Hits/', '_'+name+'_hits/'))]
    assert fields, 'No snapshot exported: '+name
    for field in fields:
        assert ak.to_list(ak.num(full[field].array(), axis=1)) == \
            ak.to_list(ak.num(upstream[field].array(), axis=1)), field
        np.testing.assert_array_equal(values(full, field), values(upstream, field),
                                      err_msg=field)
    return len(fields)


def check(full_path, upstream_path, reference_path, entries, connected_path=None):
    options = dict(handler=uproot.source.file.MemmapSource)
    with uproot.open(full_path, **options) as full_file, \
            uproot.open(upstream_path, **options) as upstream_file, \
            uproot.open(reference_path, **options) as reference_file:
        full, upstream, reference = (f['events'] for f in
                                     (full_file, upstream_file, reference_file))
        assert full.num_entries == upstream.num_entries == len(entries)
        assert len(full.keys()) == len(set(full.keys())), 'Duplicate ROOT branches'
        timing = compare_snapshot(full,upstream,'EcalClusterTiming')
        middle = 0
        if connected_path:
            with uproot.open(connected_path,**options) as connected_file:
                connected = connected_file['events']
                assert connected.num_entries == full.num_entries
                middle = compare_snapshot(full,connected,'TrkMergedECAL')
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
        metadata=full_file['podio_metadata']
        ids=dict(zip(metadata['events___idTable/m_names'].array().to_list()[0],
                     metadata['events___idTable/m_collectionIDs'].array().to_list()[0]))
        for name in ('EcalClusterTiming','TrkMergedECAL'):
            for collection in (name,name+'Hits'):
                energy=values(full,collection+'/'+collection+'.energy')
                assert len(energy) and np.all(np.isfinite(energy)) and np.all(energy>=0)
            index=full['_'+name+'_hits/_'+name+'_hits.index'].array()
            target=full['_'+name+'_hits/_'+name+'_hits.collectionID'].array()
            for event in range(full.num_entries):
                nhits=len(full[name+'Hits/'+name+'Hits.energy'].array()[event])
                assert np.all(ak.to_numpy(index[event])>=0)
                assert np.all(ak.to_numpy(index[event])<nhits)
                assert np.all(ak.to_numpy(target[event])==ids[name+'Hits'])
        print(f'PASS: {full.num_entries} events; {timing} timing fields match the '
              f'upstream-only run; {middle} middle fields match the connected-only run; '
              f'{len(final)} final PFA fields match the original rec; snapshot hit links valid.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('full')
    parser.add_argument('upstream')
    parser.add_argument('reference')
    parser.add_argument('--entries', required=True, help='Original zero-based entries, comma-separated')
    parser.add_argument('--connected',help='Same events stopped after TrackClusterConnectingAlg')
    args = parser.parse_args()
    check(args.full,args.upstream,args.reference,list(map(int,args.entries.split(','))),args.connected)
