#!/usr/bin/env python3
"""Compare passive ECAL measurements/links to REC and optional old fit fields."""
import argparse
import math
import awkward as ak
import uproot


def equal(a, b):
    if isinstance(a, list):
        return isinstance(b, list) and len(a) == len(b) and all(equal(x, y) for x, y in zip(a, b))
    return a == b or (isinstance(a, float) and isinstance(b, float) and math.isnan(a) and math.isnan(b))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rec')
    parser.add_argument('tuple')
    parser.add_argument('--baseline')
    args = parser.parse_args()
    with uproot.open(args.rec, handler=uproot.source.file.MemmapSource) as rec, \
            uproot.open(args.tuple, handler=uproot.source.file.MemmapSource) as output:
        tree = output['gsf_tuple']
        count = tree.num_entries
        metadata = rec['podio_metadata']
        ids = dict(zip(metadata['events___idTable/m_names'].array().to_list()[0],
                       metadata['events___idTable/m_collectionIDs'].array().to_list()[0]))
        cache = {}

        def field(collection, name):
            key = collection + '/' + collection + '.' + name
            if key not in cache:
                cache[key] = rec['events'][key].array(entry_stop=count).to_list()
            return cache[key]

        def links(collection, relation, event, index):
            begin = field(collection, relation+'_begin')[event][index]
            end = field(collection, relation+'_end')[event][index]
            target = '_'+collection+'_'+relation
            return list(zip(field(target, 'collectionID')[event][begin:end],
                            field(target, 'index')[event][begin:end]))

        values = {key: tree[key].array().to_list() for key in tree.keys() if key.startswith('pre_ecal_')}
        assert len(values) == 12, values.keys()
        ecal, pfos = 'EcalClusterPreReclustering', 'CyberPFOPreReclustering'
        total_clusters = total_links = 0
        for event in range(count):
            assert values['pre_ecal_available'][event] == 1
            energy = field(ecal, 'energy')[event]
            assert values['pre_ecal_cluster_index'][event] == list(range(len(energy)))
            for dest, source in [('energy', 'energy'), ('x', 'position.x'),
                                 ('y', 'position.y'), ('z', 'position.z')]:
                assert equal(values['pre_ecal_cluster_'+dest][event], field(ecal, source)[event]), (event, dest)
            assert values['pre_ecal_cluster_hit_count'][event] == [
                end-begin for begin, end in zip(field(ecal, 'hits_begin')[event], field(ecal, 'hits_end')[event])]
            assert equal(values['pre_ecal_pfo_charge'][event], field(pfos, 'charge')[event])
            track_edges, cluster_edges = [], []
            for pfo in range(len(field(pfos, 'charge')[event])):
                for cid, index in links(pfos, 'tracks', event, pfo):
                    assert cid == ids['CompleteTracks']
                    track_edges.append((pfo, index))
                for cid, index in links(pfos, 'clusters', event, pfo):
                    if cid == ids[ecal]:
                        cluster_edges.append((pfo, index))
            for kind, expected, index_name in [('track', track_edges, 'input_index'),
                                              ('cluster', cluster_edges, 'index')]:
                actual = list(zip(values['pre_ecal_pfo_'+kind+'_pfo_index'][event],
                                  values['pre_ecal_pfo_'+kind+'_'+index_name][event]))
                assert actual == expected, (event, kind)
            total_clusters += len(energy)
            total_links += len(cluster_edges)
        baseline_fields = 0
        if args.baseline:
            with uproot.open(args.baseline, handler=uproot.source.file.MemmapSource) as baseline:
                old = baseline['gsf_tuple']
                assert old.num_entries == count
                for key in old.keys():
                    if key.startswith('pre_ecal_') or key.startswith('npre_ecal_'):
                        continue
                    assert equal(ak.to_list(old[key].array()), ak.to_list(tree[key].array())), key
                    baseline_fields += 1
        comparison = (f'{baseline_fields} existing fields agree exactly with baseline'
                      if args.baseline else 'baseline comparison not requested')
        print(f'PASS: {count} events, {total_clusters} ECAL clusters, {total_links} PFO/ECAL links; {comparison}')


if __name__ == '__main__':
    main()
