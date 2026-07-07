#!/usr/bin/env python3
import argparse
import csv
import math
from pathlib import Path

import numpy as np
import uproot

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

B_FIELD_T = 3.0
OMEGA_FACTOR = 0.0003 * B_FIELD_T  # omega[1/mm] = q * 0.3 B[T] / pT[GeV] / 1000

BRANCHES = [
    'CompleteTracks/CompleteTracks.trackStates_begin',
    'CompleteTracks/CompleteTracks.trackStates_end',
    'CompleteTracks/CompleteTracks.chi2',
    'CompleteTracks/CompleteTracks.ndf',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.location',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.D0',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.phi',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.omega',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.Z0',
    '_CompleteTracks_trackStates/_CompleteTracks_trackStates.tanLambda',
    'MCParticleGen/MCParticleGen.PDG',
    'MCParticleGen/MCParticleGen.momentum.x',
    'MCParticleGen/MCParticleGen.momentum.y',
    'MCParticleGen/MCParticleGen.momentum.z',
    'CompleteTracksParticleAssociation/CompleteTracksParticleAssociation.weight',
    '_CompleteTracksParticleAssociation_rec/_CompleteTracksParticleAssociation_rec.index',
    '_CompleteTracksParticleAssociation_sim/_CompleteTracksParticleAssociation_sim.index',
]

TAIL_THRESHOLDS = {
    'd0_mm': [0.1, 1.0, 10.0],
    'z0_mm': [0.1, 1.0, 10.0],
    'phi_mrad': [1.0, 10.0, 100.0],
    'omega_rel_pct': [1.0, 10.0, 100.0],
    'pt_rel_pct': [1.0, 10.0, 50.0],
}

PARAMS = [
    ('d0_mm', 'D0 [mm]', (-0.20, 0.20)),
    ('z0_mm', 'Z0 [mm]', (-0.20, 0.20)),
    ('phi_mrad', 'phi residual [mrad]', (-8.0, 8.0)),
    ('tanlambda', 'tan(lambda) residual', (-0.004, 0.004)),
    ('omega_rel_pct', 'omega relative residual [%]', (-15.0, 15.0)),
    ('pt_rel_pct', 'pT relative residual [%]', (-2.0, 2.0)),
]

COLORS = {'electron': 'black', 'muon': '#0072B2'}


def delta_phi(a, b):
    d = a - b
    while d > math.pi:
        d -= 2.0 * math.pi
    while d <= -math.pi:
        d += 2.0 * math.pi
    return d


def charge_from_pdg(pdg):
    # Sign convention for these samples: PDG 11 and 13 are negatively charged particles.
    if abs(pdg) in (11, 13):
        return -1.0 if pdg > 0 else 1.0
    return 0.0


def as_event_list(x):
    return x


def choose_track(arr, ev):
    starts = arr['CompleteTracks/CompleteTracks.trackStates_begin'][ev]
    ends = arr['CompleteTracks/CompleteTracks.trackStates_end'][ev]
    if len(starts) == 0:
        return None

    rec = arr['_CompleteTracksParticleAssociation_rec/_CompleteTracksParticleAssociation_rec.index'][ev]
    sim = arr['_CompleteTracksParticleAssociation_sim/_CompleteTracksParticleAssociation_sim.index'][ev]
    weights = arr['CompleteTracksParticleAssociation/CompleteTracksParticleAssociation.weight'][ev]
    best_track = None
    best_weight = -1.0
    for r, s, w in zip(rec, sim, weights):
        if int(s) == 0 and int(r) >= 0 and int(r) < len(starts) and float(w) > best_weight:
            best_track = int(r)
            best_weight = float(w)
    if best_track is not None:
        return best_track

    return int(np.argmax(arr['CompleteTracks/CompleteTracks.ndf'][ev]))


def ip_state_index(arr, ev, track_index):
    starts = arr['CompleteTracks/CompleteTracks.trackStates_begin'][ev]
    ends = arr['CompleteTracks/CompleteTracks.trackStates_end'][ev]
    loc = arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.location'][ev]
    b, e = int(starts[track_index]), int(ends[track_index])
    for i in range(b, e):
        if int(loc[i]) == 1:
            return i
    return b if e > b else None


def analyze_sample(label, pattern):
    rows = []
    files = [pattern.format(i=i) for i in range(1, 11)]
    for file_index, path in enumerate(files, start=1):
        f = uproot.open(path)
        tree = f['events']
        arr = tree.arrays(BRANCHES, library='np')
        nentries = tree.num_entries
        for ev in range(nentries):
            pdgs = arr['MCParticleGen/MCParticleGen.PDG'][ev]
            if len(pdgs) == 0:
                continue
            pdg = int(pdgs[0])
            q = charge_from_pdg(pdg)
            px = float(arr['MCParticleGen/MCParticleGen.momentum.x'][ev][0])
            py = float(arr['MCParticleGen/MCParticleGen.momentum.y'][ev][0])
            pz = float(arr['MCParticleGen/MCParticleGen.momentum.z'][ev][0])
            pt = math.hypot(px, py)
            if pt <= 0 or q == 0:
                continue
            truth_phi = math.atan2(py, px)
            truth_tanl = pz / pt
            truth_omega = q * OMEGA_FACTOR / pt

            trk = choose_track(arr, ev)
            if trk is None:
                continue
            st = ip_state_index(arr, ev, trk)
            if st is None:
                continue

            d0 = float(arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.D0'][ev][st])
            z0 = float(arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.Z0'][ev][st])
            phi = float(arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.phi'][ev][st])
            omega = float(arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.omega'][ev][st])
            tanl = float(arr['_CompleteTracks_trackStates/_CompleteTracks_trackStates.tanLambda'][ev][st])
            chi2 = float(arr['CompleteTracks/CompleteTracks.chi2'][ev][trk])
            ndf = int(arr['CompleteTracks/CompleteTracks.ndf'][ev][trk])
            pt_rec = abs(OMEGA_FACTOR / omega) if omega != 0 else float('nan')

            rows.append({
                'sample': label,
                'file_index': file_index,
                'event': ev,
                'track_index': trk,
                'pdg': pdg,
                'truth_pt_GeV': pt,
                'truth_phi': truth_phi,
                'truth_tanlambda': truth_tanl,
                'truth_omega': truth_omega,
                'd0_mm': d0,
                'z0_mm': z0,
                'phi_mrad': 1000.0 * delta_phi(phi, truth_phi),
                'tanlambda': tanl - truth_tanl,
                'omega_rel_pct': 100.0 * (omega - truth_omega) / truth_omega,
                'pt_rel_pct': 100.0 * (pt_rec - pt) / pt,
                'chi2': chi2,
                'ndf': ndf,
                'chi2_ndf': chi2 / ndf if ndf > 0 else float('nan'),
            })
    return rows


def stats(values):
    a = np.asarray(values, dtype=float)
    a = a[np.isfinite(a)]
    if len(a) == 0:
        return {'count': 0, 'mean': float('nan'), 'std': float('nan'), 'median': float('nan'), 'q16': float('nan'), 'q84': float('nan'), 'rms': float('nan')}
    return {
        'count': int(len(a)),
        'mean': float(np.mean(a)),
        'std': float(np.std(a, ddof=1)) if len(a) > 1 else 0.0,
        'median': float(np.median(a)),
        'q16': float(np.quantile(a, 0.16)),
        'q84': float(np.quantile(a, 0.84)),
        'rms': float(math.sqrt(np.mean(a * a))),
    }


def plot_param(rows_by_sample, key, xlabel, xlim, outdir):
    fig, ax = plt.subplots(figsize=(8.5, 6.0))
    bins = np.linspace(xlim[0], xlim[1], 90)
    for label, rows in rows_by_sample.items():
        vals = np.asarray([r[key] for r in rows], dtype=float)
        vals = vals[np.isfinite(vals)]
        ax.hist(vals, bins=bins, histtype='step', density=True, linewidth=2.0,
                color=COLORS[label], label=f'{label} (N={len(vals)})')
    ax.set_xlabel(xlabel)
    ax.set_ylabel('normalized tracks')
    ax.set_xlim(*xlim)
    ax.grid(True, alpha=0.25)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(outdir / f'{key}_resolution_comparison.png', dpi=160)
    fig.savefig(outdir / f'{key}_resolution_comparison.pdf')
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description='Compare LCIO CompleteTracks IP-state resolutions for 2 GeV theta=85 electron and muon samples.')
    parser.add_argument('--outdir', default='TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85')
    args = parser.parse_args()

    outdir = Path(args.outdir)
    plotdir = outdir / 'plots'
    outdir.mkdir(parents=True, exist_ok=True)
    plotdir.mkdir(parents=True, exist_ok=True)

    rows_by_sample = {
        'electron': analyze_sample('electron', 'trk-e--2.0-85-{i}.root'),
        'muon': analyze_sample('muon', 'trk-mu--2.0-85-{i}.root'),
    }
    all_rows = rows_by_sample['electron'] + rows_by_sample['muon']

    with open(outdir / 'track_resolution_rows.csv', 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(all_rows[0].keys()))
        writer.writeheader()
        writer.writerows(all_rows)

    summary_rows = []
    for label, rows in rows_by_sample.items():
        for key, _, _ in PARAMS + [('chi2_ndf', 'chi2/ndf', (0, 3))]:
            st = stats([r[key] for r in rows])
            summary_rows.append({'sample': label, 'quantity': key, **st})
    with open(outdir / 'track_resolution_summary.csv', 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(summary_rows[0].keys()))
        writer.writeheader()
        writer.writerows(summary_rows)

    tail_rows = []
    for label, rows in rows_by_sample.items():
        n = len(rows)
        for key, thresholds in TAIL_THRESHOLDS.items():
            vals = np.asarray([abs(r[key]) for r in rows], dtype=float)
            vals = vals[np.isfinite(vals)]
            for threshold in thresholds:
                count = int(np.sum(vals > threshold))
                tail_rows.append({
                    'sample': label,
                    'quantity': key,
                    'abs_threshold': threshold,
                    'count': count,
                    'fraction': count / n if n else float('nan'),
                })
    with open(outdir / 'track_resolution_tail_summary.csv', 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(tail_rows[0].keys()))
        writer.writeheader()
        writer.writerows(tail_rows)

    with open(outdir / 'summary.txt', 'w') as f:
        f.write('LCIO CompleteTracks resolution comparison: 2 GeV, theta=85 deg\n')
        f.write('Inputs: trk-e--2.0-85-{1..10}.root and trk-mu--2.0-85-{1..10}.root\n')
        f.write('Track: CompleteTracks, matched to primary MCParticle through CompleteTracksParticleAssociation when possible.\n')
        f.write('State: LCIO TrackState with location == 1, reference point at IP.\n')
        f.write(f'Assumed magnetic field for omega/pT conversion: {B_FIELD_T:.1f} T.\n')
        f.write('Truth: MCParticleGen[0], generated particle momentum. D0 and Z0 truth are taken as 0 for particle-gun IP samples.\n\n')
        for row in summary_rows:
            f.write(f"{row['sample']:8s} {row['quantity']:14s} count {row['count']:4d} mean {row['mean']:.8g} std {row['std']:.8g} median {row['median']:.8g} q16 {row['q16']:.8g} q84 {row['q84']:.8g} rms {row['rms']:.8g}\n")
        f.write('\nTail fractions use absolute residual thresholds.\n')
        for row in tail_rows:
            f.write(f"{row['sample']:8s} {row['quantity']:14s} |x| > {row['abs_threshold']:<8g} count {row['count']:4d} fraction {row['fraction']:.6f}\n")

    for key, xlabel, xlim in PARAMS:
        plot_param(rows_by_sample, key, xlabel, xlim, plotdir)
    plot_param(rows_by_sample, 'chi2_ndf', 'chi2/ndf', (0.0, 2.5), plotdir)

if __name__ == '__main__':
    main()
