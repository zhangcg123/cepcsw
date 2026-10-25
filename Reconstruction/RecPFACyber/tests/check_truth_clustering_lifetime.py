"""Run in the CEPCSW environment; optionally select a built PFA module."""
import argparse
from pathlib import Path

import ROOT


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--library', default='libCrystalCaloRec.so')
args = parser.parse_args()
assert ROOT.gSystem.Load(args.library) >= 0, args.library
ROOT.gInterpreter.AddIncludePath(str(Path(__file__).resolve().parents[1] / 'include'))
assert ROOT.gInterpreter.Declare(r'''
#include "Algorithm/TruthClusteringAlg.h"

bool checkTruthClusteringLifetime() {
    TruthClusteringAlg algorithm;
    Cyber::Settings settings;
    if (algorithm.ReadSettings(settings).isFailure()) return false;
    for (int event = 0; event < 3; ++event) {
        CyberDataCol data;
        data.TrackCol.push_back(std::make_shared<Cyber::Track>());
        std::weak_ptr<Cyber::Track> track = data.TrackCol.front();
        if (algorithm.Initialize(data).isFailure()) return false;
        if (algorithm.ClearAlgorithm().isFailure()) return false;
        // The algorithm must not retain event objects past the owning event.
        if (track.use_count() != 1) return false;
        if (data.Clear().isFailure() || !track.expired()) return false;
        // Empty events and repeated cleanup must also be safe.
        if (algorithm.Initialize(data).isFailure()) return false;
        if (algorithm.ClearAlgorithm().isFailure()) return false;
        if (algorithm.ClearAlgorithm().isFailure()) return false;
    }
    return true;
}
''')
assert ROOT.checkTruthClusteringLifetime(), 'TruthClusteringAlg retains an event track after cleanup'
print('PASS: tracks released within their event; empty events and repeated cleanup are safe.')
