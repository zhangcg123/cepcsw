"""Check entry flags and, optionally, same-seed preservation of old truth steps.

Run with Python + uproot/awkward: check_ecal_entries.py NEW.root [OLD.root].
"""
import json
import sys
from collections import Counter

import awkward as ak
import numpy as np
import uproot

STEP = 'GsfG4MaterialSteps/GsfG4MaterialSteps.'
PHOTON = 'GsfG4BremsPhotons/GsfG4BremsPhotons.'
FLAGS = ('firstEcalEntry', 'ecalEntryOnly')


def read(path):
    with uproot.open(path, handler=uproot.source.file.MemmapSource) as root:
        tree = root['events']
        prefixes = (STEP, PHOTON, 'GsfG4BremsPhotonSteps/', 'MCParticle/',
                    '_GsfG4BremsPhotons_parentStep/')
        return tree.arrays([key for key in tree.keys() if key.startswith(prefixes)],
                           library='ak', how=dict)


def equal(a, b):
    return (ak.to_list(ak.num(a)) == ak.to_list(ak.num(b)) and
            np.array_equal(ak.to_numpy(ak.flatten(a)), ak.to_numpy(ak.flatten(b)), equal_nan=True))


def check(path, reference=None):
    new = read(path)
    first, only = (new[STEP+f] for f in FLAGS)
    assert bool(ak.all((first == 0) | (first == 1)))
    assert bool(ak.all((only == 0) | (only == 1)))
    assert bool(ak.all((only == 0) | (first == 1)))
    selected = first == 1
    counts = Counter()
    for event in range(len(first)):
        ids = ak.to_list(new[STEP+'trackID'][event][selected[event]])
        assert len(ids) == len(set(ids)), (event, 'duplicate first entry')
        counts.update(ak.to_list(new[STEP+'pdg'][event][selected[event]]))
        # Follow the relation, not old collection indices shifted by extra steps.
        indices = new['_GsfG4BremsPhotons_parentStep/_GsfG4BremsPhotons_parentStep.index'][event]
        for photon, index in enumerate(ak.to_list(indices)):
            assert new[STEP+'trackID'][event][index] == new[PHOTON+'parentTrackID'][event][photon]
            assert new[STEP+'stepNumber'][event][index] == new[PHOTON+'parentStepNumber'][event][photon]
            assert not only[event][index]
    for name in ('postPosition.x', 'postPosition.y', 'postPosition.z',
                 'postMomentum.x', 'postMomentum.y', 'postMomentum.z'):
        assert bool(ak.all(np.isfinite(new[STEP+name][selected])))
    report = dict(events=len(first), entries_by_pdg=dict(counts),
                  extra_steps=int(ak.sum(only)), total_steps=int(ak.sum(ak.num(first))))
    if reference:
        old = read(reference)
        assert len(old[STEP+'trackID']) == len(first)
        drift = {}
        for name, values in old.items():
            if not name.startswith(STEP):
                continue
            current = new[name][only == 0]
            if name.endswith(('preVolumeCopyNo', 'postVolumeCopyNo')):
                # Geant4 copy-number bookkeeping has known run-to-run drift.
                assert ak.to_list(ak.num(values)) == ak.to_list(ak.num(current))
                drift[name] = int(ak.sum(values != current))
            else:
                assert equal(values, current), name
        # New step indices change relations, not the photon birth/path values.
        checked = 0
        for name, values in old.items():
            if name.startswith(('GsfG4BremsPhotons/', 'GsfG4BremsPhotonSteps/', 'MCParticle/')):
                assert equal(values, new[name]), name
                checked += 1
        report.update(old_material_preserved=True, other_branches_checked=checked,
                      material_copy_number_drift=drift)
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    check(*sys.argv[1:])
