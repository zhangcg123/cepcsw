"""Focused snapshot ownership test; run with the CEPCSW/build environment."""
import argparse
from pathlib import Path

import ROOT

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--library', default='libCrystalCaloRec.so')
args = parser.parse_args()
assert ROOT.gSystem.Load(args.library) >= 0
ROOT.gInterpreter.AddIncludePath(str(Path(__file__).resolve().parents[1] / 'include'))
assert ROOT.gInterpreter.Declare(r'''
#include "Tools/OutputCreator.h"
#include <cassert>

bool checkPrePFOSnapshot() {
    Cyber::Settings settings;
    settings.map_stringPars["OutputPFO"] = "outputPFO";
    settings.map_boolPars["UseTruthTrk"] = false;
    settings.map_boolPars["UseTruthMatchTrk"] = false;
    settings.map_floatPars["ECALNeutralCalib"] = 1.;
    settings.map_floatPars["HCALNeutralCalib"] = 4.;
    Cyber::OutputCreator exporter(settings);
    edm4hep::TrackCollection originalTracks;
    originalTracks.setID(10);
    Cyber::Track track1, track2;
    auto first = edm4hep::Track(originalTracks.create());
    auto second = edm4hep::Track(originalTracks.create());
    track1.setOriginTrack(first);
    track2.setOriginTrack(second);
    Cyber::TrackState state{};
    state.location = Cyber::TrackState::AtIP;
    state.Kappa = .1;
    state.Omega = .001;
    std::vector<Cyber::TrackState> states{state};
    track1.setTrackStates("Input", states);
    track2.setTrackStates("Input", states);

    edm4hep::CalorimeterHitCollection originals;
    auto originalHit = edm4hep::CalorimeterHit(originals.create());
    Cyber::CaloHit hit;
    hit.setOriginHit(originalHit);
    hit.setEnergy(2.);
    hit.setPosition({2000., 0., 100.});
    Cyber::Calo3DCluster cluster;
    cluster.setCaloHits({&hit});
    auto charged = std::make_shared<Cyber::PFObject>();
    charged->setPID(11);
    charged->addTrack(&track1);
    charged->addTrack(&track2);
    charged->addHCALCluster(&cluster);
    auto neutral = std::make_shared<Cyber::PFObject>();
    neutral->addHCALCluster(&cluster);
    auto trackOnly = std::make_shared<Cyber::PFObject>();
    trackOnly->setPID(11);
    trackOnly->addTrack(&track2);
    CyberDataCol data;
    data.map_PFObjects["outputPFO"] = {charged, neutral, trackOnly};
    edm4hep::ReconstructedParticleCollection pfos;
    edm4hep::ClusterCollection ecal, hcal;
    edm4hep::CalorimeterHitCollection ecalHits, hcalHits;
    edm4hep::TrackCollection truthTracks;
    Cyber::ClusterTruthCollection ecalTruth;
    assert(exporter.CreatePFOSnapshot(data, pfos, ecal, ecalHits,
                                      hcal, hcalHits, truthTracks, ecalTruth).isSuccess());
    assert(pfos.size() == 3 && ecal.empty() && ecalHits.empty());
    assert(hcal.size() == 1 && hcalHits.size() == 1 && truthTracks.empty());
    assert(pfos[0].tracks_size() == 2 && pfos[1].tracks_size() == 0);
    assert(pfos[0].getTracks(0) == first && pfos[0].getTracks(1) == second);
    assert(pfos[2].tracks_size() == 1 && pfos[2].clusters_size() == 0);
    assert(pfos[0].getClusters(0) == pfos[1].getClusters(0));
    assert(pfos[1].getEnergy() == 8. && pfos[1].getCharge() == 0.);
    assert(hcal[0].getEnergy() == 2. && hcal[0].getHits(0).getEnergy() == 2.);
    assert(hcal[0].getPosition().x == 2000.);
    assert(hit.getEnergy() == 2. && charged->getTracks().size() == 2);
    // Simulate downstream in-place changes and deletion of the source PFO list.
    hit.setEnergy(99.);
    cluster.setEnergyScale(5.);
    charged->setTrack({});
    charged->setHCALCluster({});
    data.map_PFObjects["outputPFO"].clear();
    assert(hcal[0].getEnergy() == 2. && hcalHits[0].getEnergy() == 2.);
    assert(pfos[0].tracks_size() == 2 && pfos[0].clusters_size() == 1);
    assert(pfos[1].getEnergy() == 8.);
    edm4hep::ReconstructedParticleCollection empty;
    assert(exporter.CreatePFOSnapshot(data, empty, ecal, ecalHits,
                                      hcal, hcalHits, truthTracks, ecalTruth).isSuccess());
    assert(empty.empty());
    edm4hep::MCParticleCollection particles;
    particles.setID(30);
    auto photon = edm4hep::MCParticle(particles.create());
    auto other = edm4hep::MCParticle(particles.create());
    Cyber::CaloUnit bar;
    bar.setQ(1., 2.);
    Cyber::Calo1DCluster shower;
    shower.setBars({&bar});
    Cyber::CaloHalfCluster half;
    half.addUnit(&shower);
    Cyber::Calo2DCluster plane;
    plane.addShowerU(&shower);
    plane.setPos(2000., 0., 100.);
    Cyber::Calo3DCluster ecalSource;
    ecalSource.setClusters({&plane});
    ecalSource.addHalfClusterU("LinkedLongiCluster", &half);
    ecalSource.setLinkedMCP({{photon, .8f}, {other, .2f}});
    auto one = std::make_shared<Cyber::PFObject>();
    auto two = std::make_shared<Cyber::PFObject>();
    one->addECALCluster(&ecalSource);
    two->addECALCluster(&ecalSource);
    data.map_PFObjects["outputPFO"] = {one, two};
    ecal.setID(31);
    assert(exporter.CreatePFOSnapshot(data, empty, ecal, ecalHits,
                                      hcal, hcalHits, truthTracks, ecalTruth).isSuccess());
    assert(ecal.size() == 1 && ecalTruth.size() == 2);
    assert(ecalTruth[0].getWeight() == .8f && ecalTruth[1].getWeight() == .2f);
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    assert(ecalTruth[0].getFrom() == ecal[0] && ecalTruth[0].getTo() == photon);
    assert(ecalTruth[1].getFrom() == ecal[0] && ecalTruth[1].getTo() == other);
#else
    assert(ecalTruth[0].getRec() == ecal[0] && ecalTruth[0].getSim() == photon);
    assert(ecalTruth[1].getRec() == ecal[0] && ecalTruth[1].getSim() == other);
#endif
    assert(ecalSource.getLinkedMCP().size() == 2);
    ecalSource.setLinkedMCP({});
    assert(ecalTruth[0].getWeight() == .8f);
    return true;
}
''')
assert ROOT.checkPrePFOSnapshot()
print('PASS: original track links, shared cluster/hit identity, trackless/track-only '
      'PFOs, independent energies after source mutation, empty events, and '
      'deduplicated ECAL truth links with direct MC references and immutable weights.')
