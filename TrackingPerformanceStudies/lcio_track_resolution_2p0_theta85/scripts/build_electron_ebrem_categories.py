#!/usr/bin/env python3
import argparse
import json
import math
from pathlib import Path

import numpy as np
import uproot

TRACKER_TOKENS = ("VXD", "ITK", "TPC", "OTK", "SIT", "SET")
BRANCHES = [
    'event_id',
    'track_id',
    'parent_id',
    'pdg',
    'process_subtype',
    'pre_p',
    'post_p',
    'loss',
    'pre_volume',
]


def is_tracker_volume(volume):
    return any(tok in str(volume) for tok in TRACKER_TOKENS)


def summarize_event(values, hard_single_frac, hard_cumulative_frac):
    event_id = int(values['event_id'])
    track_id = values['track_id']
    parent_id = values['parent_id']
    pdg = values['pdg']
    subtype = values['process_subtype']
    pre_p = values['pre_p']
    post_p = values['post_p']
    loss = values['loss']
    pre_volume = values['pre_volume']

    count = 0
    tracker_count = 0
    max_frac = 0.0
    max_tracker_frac = 0.0
    sum_loss = 0.0
    tracker_sum_loss = 0.0
    retained_product = 1.0
    tracker_retained_product = 1.0

    for i, proc in enumerate(subtype):
        if int(proc) != 3:
            continue
        if int(track_id[i]) != 1 or int(parent_id[i]) != 0 or int(pdg[i]) != 11:
            continue
        p0 = float(pre_p[i])
        p1 = float(post_p[i])
        if p0 <= 0 or not math.isfinite(p0) or not math.isfinite(p1):
            continue
        retained = max(0.0, min(1.0, p1 / p0))
        frac = 1.0 - retained
        step_loss = float(loss[i]) if math.isfinite(float(loss[i])) else 0.0
        count += 1
        max_frac = max(max_frac, frac)
        sum_loss += max(0.0, step_loss)
        retained_product *= retained
        if is_tracker_volume(pre_volume[i]):
            tracker_count += 1
            max_tracker_frac = max(max_tracker_frac, frac)
            tracker_sum_loss += max(0.0, step_loss)
            tracker_retained_product *= retained

    cumulative_frac = 1.0 - retained_product if count else 0.0
    tracker_cumulative_frac = 1.0 - tracker_retained_product if tracker_count else 0.0
    if count == 0:
        category = 'no_ebrem'
    elif max_frac >= hard_single_frac or cumulative_frac >= hard_cumulative_frac:
        category = 'hard_ebrem'
    else:
        category = 'light_ebrem'

    if tracker_count == 0:
        tracker_category = 'no_tracker_ebrem'
    elif max_tracker_frac >= hard_single_frac or tracker_cumulative_frac >= hard_cumulative_frac:
        tracker_category = 'hard_tracker_ebrem'
    else:
        tracker_category = 'light_tracker_ebrem'

    return {
        'event_id': event_id,
        'primary_ebrem_count': count,
        'primary_tracker_ebrem_count': tracker_count,
        'max_single_frac_loss': max_frac,
        'max_tracker_single_frac_loss': max_tracker_frac,
        'cumulative_frac_loss': cumulative_frac,
        'tracker_cumulative_frac_loss': tracker_cumulative_frac,
        'sum_loss_GeV': sum_loss,
        'tracker_sum_loss_GeV': tracker_sum_loss,
        'category': category,
        'tracker_category': tracker_category,
    }


def main():
    parser = argparse.ArgumentParser(description='Build event-level eBrem categories from electron material-step tuples.')
    parser.add_argument('--pattern', default='gsf_material_steps-e--2.0-85-{i}.root')
    parser.add_argument('--out', default='TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/electron_ebrem_event_categories.json')
    parser.add_argument('--hard-single-frac', type=float, default=0.10)
    parser.add_argument('--hard-cumulative-frac', type=float, default=0.10)
    args = parser.parse_args()

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    events = []
    counts = {}
    tracker_counts = {}
    for file_index in range(1, 11):
        path = args.pattern.format(i=file_index)
        with uproot.open(path) as f:
            tree = f['g4step_tuple']
            arrays = tree.arrays(BRANCHES, library='np')
            for entry in range(tree.num_entries):
                values = {name: arrays[name][entry] for name in BRANCHES}
                rec = summarize_event(values, args.hard_single_frac, args.hard_cumulative_frac)
                rec['file_index'] = file_index
                rec['entry_index'] = entry
                rec['tracking_file'] = f'trk-e--2.0-85-{file_index}.root'
                rec['material_step_file'] = path
                events.append(rec)
                counts[rec['category']] = counts.get(rec['category'], 0) + 1
                tracker_counts[rec['tracker_category']] = tracker_counts.get(rec['tracker_category'], 0) + 1

    payload = {
        'schema': 'electron_ebrem_event_categories_v1',
        'description': 'Event-level primary-electron eBrem categories built from gsf_material_steps-e--2.0-85 material-step tuples. Join to tracking rows by file_index and entry_index/event index.',
        'selection': 'track_id == 1 && parent_id == 0 && pdg == 11 && process_subtype == 3',
        'tracker_volume_tokens': list(TRACKER_TOKENS),
        'hard_definition': {
            'hard_single_frac_loss_threshold': args.hard_single_frac,
            'hard_cumulative_frac_loss_threshold': args.hard_cumulative_frac,
            'hard_if': 'max_single_frac_loss >= threshold OR cumulative_frac_loss >= threshold',
            'frac_loss': '1 - post_p/pre_p',
        },
        'counts': counts,
        'tracker_counts': tracker_counts,
        'n_events': len(events),
        'events': events,
    }
    out.write_text(json.dumps(payload, indent=2, sort_keys=True))
    print(f'wrote {out}')
    print('counts', counts)
    print('tracker_counts', tracker_counts)

if __name__ == '__main__':
    main()
