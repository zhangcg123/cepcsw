"""Validate full-chain pre-PFO snapshots against a connected-only run and old output."""
import argparse
import awkward as ak
import numpy as np
import uproot

PFO = 'CyberPFOPreReclustering'
ECAL = 'EcalClusterPreReclustering'
HCAL = 'HcalClusterPreReclustering'
EHITS = 'EcalHitsPreReclustering'
HHITS = 'HcalHitsPreReclustering'


class EventFile:
    def __init__(self, path):
        self.file = uproot.open(path, handler=uproot.source.file.MemmapSource)
        self.tree = self.file['events']
        table = self.file['podio_metadata']
        self.ids = dict(zip(table['events___idTable/m_names'].array().to_list()[0],
                            table['events___idTable/m_collectionIDs'].array().to_list()[0]))
        self.names = {value: key for key, value in self.ids.items()}
        self.cache = {}

    def get(self, collection, field, event):
        key = collection + '/' + collection + '.' + field
        if key not in self.cache:
            self.cache[key] = self.tree[key].array().to_list()
        return self.cache[key][event]

    def links(self, collection, relation, event, index):
        begin = self.get(collection, relation + '_begin', event)[index]
        end = self.get(collection, relation + '_end', event)[index]
        target = '_' + collection + '_' + relation
        ids = self.get(target, 'collectionID', event)[begin:end]
        indices = self.get(target, 'index', event)[begin:end]
        result = []
        for cid, idx in zip(ids, indices):
            name = self.names[cid]
            count_field = 'chi2' if name in ('CompleteTracks', 'TracksPreReclustering') else 'energy'
            assert 0 <= idx < len(self.get(name, count_field, event)), (name, idx)
            result.append((name, idx))
        return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('full')
    parser.add_argument('connected')
    parser.add_argument('reference')
    args = parser.parse_args()
    full, connected, reference = [EventFile(p) for p in
                                   (args.full, args.connected, args.reference)]
    assert full.tree.num_entries == connected.tree.num_entries == reference.tree.num_entries
    names = (PFO, ECAL, HCAL, EHITS, HHITS, 'TracksPreReclustering')
    fields = [key for key in full.tree.keys() if any(
        key.startswith((name + '/', '_' + name + '_')) for name in names)]
    assert fields
    for key in fields:
        a, b = full.tree[key].array(), connected.tree[key].array()
        assert ak.to_list(ak.num(a, axis=1)) == ak.to_list(ak.num(b, axis=1)), key
        np.testing.assert_array_equal(ak.to_numpy(ak.flatten(a, axis=None)),
                                      ak.to_numpy(ak.flatten(b, axis=None)), err_msg=key)

    counts = dict(pfos=0, charged=0, trackless=0, track_only=0, ecal=0, hcal=0,
                  ecal_hits=0, hcal_hits=0, changed_final_pfo_counts=0)
    for event in range(full.tree.num_entries):
        n = len(full.get(PFO, 'energy', event))
        assert n == len(connected.get('CyberPFO', 'energy', event))
        counts['pfos'] += n
        counts['changed_final_pfo_counts'] += n != len(full.get('CyberPFO', 'energy', event))
        for name, count in ((ECAL, 'ecal'), (HCAL, 'hcal'),
                            (EHITS, 'ecal_hits'), (HHITS, 'hcal_hits')):
            energy = full.get(name, 'energy', event)
            assert all(np.isfinite(x) and x >= 0. for x in energy), name
            counts[count] += len(energy)
        for cluster, hitname in ((ECAL, EHITS), (HCAL, HHITS)):
            for i in range(len(full.get(cluster, 'energy', event))):
                links = full.links(cluster, 'hits', event, i)
                assert links and all(name == hitname for name, _ in links)
        for i in range(n):
            tracks = full.links(PFO, 'tracks', event, i)
            assert tracks == connected.links('CyberPFO', 'tracks', event, i)
            counts['charged' if tracks else 'trackless'] += 1
            clusters = full.links(PFO, 'clusters', event, i)
            counts['track_only'] += bool(tracks) and not clusters
            original = connected.links('CyberPFO', 'clusters', event, i)
            assert len(clusters) == len(original)
            np.testing.assert_allclose(full.get(PFO, 'energy', event)[i],
                                       connected.get('CyberPFO', 'energy', event)[i], rtol=2e-6)
            for field in ('charge', 'mass', 'momentum.x', 'momentum.y', 'momentum.z'):
                np.testing.assert_allclose(full.get(PFO, field, event)[i],
                                           connected.get('CyberPFO', field, event)[i], rtol=2e-6,
                                           atol=1e-6, err_msg=field)
            ecal_raw = sum(full.get(name, 'energy', event)[j]
                           for name, j in clusters if name == ECAL)
            p = np.linalg.norm([connected.get('CyberPFO', 'momentum.'+c, event)[i]
                                for c in 'xyz'])
            for (name, j), (oldname, k) in zip(clusters, original):
                assert oldname == ('EcalCluster' if name == ECAL else 'HcalCluster')
                # Maintained card: standard output calibration, absent from raw snapshots.
                scale = 4. if name == HCAL else (
                    1.26 if tracks and (p < 15. or ecal_raw / p < .8) else 1.)
                np.testing.assert_allclose(full.get(name, 'energy', event)[j] * scale,
                    connected.get(oldname, 'energy', event)[k], rtol=2e-6)
                for c in 'xyz':
                    assert full.get(name, 'position.'+c, event)[j] == \
                        connected.get(oldname, 'position.'+c, event)[k]
        # The pre-PFO ECAL objects must reproduce the existing passive merged snapshot.
        def fingerprints(cluster):
            result = []
            for i, energy in enumerate(full.get(cluster, 'energy', event)):
                hits = []
                for name, j in full.links(cluster, 'hits', event, i):
                    hits.append(tuple(full.get(name, field, event)[j] for field in
                                      ('energy', 'position.x', 'position.y', 'position.z', 'cellID', 'type')))
                result.append((energy, *(full.get(cluster, 'position.'+c, event)[i] for c in 'xyz'),
                               tuple(sorted(hits))))
            return sorted(result)
        assert fingerprints(ECAL) == fingerprints('TrkMergedECAL'), event

    # Existing final outputs, including all relation indices, must be untouched.
    prefixes = ('CyberPFO/', 'CyberPFOPID/', 'EcalCluster/', 'EcalCore/', 'HcalCluster/',
                'RecECALBarrel/', 'RecECALBarrelCore/', 'RecHCALBarrel/',
                '_CyberPFO_', '_CyberPFOPID_', '_EcalCluster_', '_EcalCore_', '_HcalCluster_',
                'ParticleID/', '_ParticleID_')
    old_fields = [key for key in reference.tree.keys() if key.startswith(prefixes)]
    for key in old_fields:
        a, b = full.tree[key].array(), reference.tree[key].array()
        assert ak.to_list(ak.num(a, axis=1)) == ak.to_list(ak.num(b, axis=1)), key
        np.testing.assert_array_equal(ak.to_numpy(ak.flatten(a, axis=None)),
                                      ak.to_numpy(ak.flatten(b, axis=None)), err_msg=key)
    print(f'PASS: {full.tree.num_entries} events; {len(fields)} pre-reclustering fields '
          f'exactly match the stage-stopped run; {len(old_fields)} existing final fields '
          f'exactly match the old runtime. Valid tracks/clusters/hits and energy conventions. {counts}')


if __name__ == '__main__':
    main()
