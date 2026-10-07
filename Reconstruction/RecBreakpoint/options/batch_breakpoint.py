"""Batch plumbing only: freeze cards, submit manifests, run and verify outputs.

The fitter is configured in run_breakpoint.py. No GSF algorithm is executed.
Prepare uses only the Python standard library; ROOT is loaded only by workers.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys

ORDER = ('sim', 'trk', 'calodigi', 'rec', 'breakpoint')
SIM_CALO_COLLECTIONS = (
    'EcalBarrelCollection', 'EcalBarrelContributionCollection',
    'EcalEndcapsCollection', 'EcalEndcapsContributionCollection',
    'HcalBarrelCollection', 'HcalBarrelContributionCollection',
    'HcalEndcapsCollection', 'HcalEndcapsContributionCollection',
)
SIM_BREMS_COLLECTIONS = ('GsfG4BremsPhotons', 'GsfG4BremsPhotonSteps')
BREAKPOINT_INPUT_COLLECTIONS = (
    'MCParticle', 'CompleteTracks', 'CompleteTracksParticleAssociation',
    'GsfG4BremsPhotons', 'GsfG4BremsPhotonSteps',
    'VXDTrackerHits', 'ITKBarrelTrackerHits', 'ITKEndcapTrackerHits',
    'TPCTrackerHits', 'OTKBarrelTrackerHits', 'OTKEndcapTrackerHits',
    'VXDTrackerHitAssociation', 'ITKBarrelTrackerHitAssociation',
    'ITKEndcapTrackerHitAssociation', 'TPCTrackerHitAss',
    'OTKBarrelTrackerHitAssociation', 'OTKEndcapTrackerHitAssociation',
    'GsfG4MaterialSteps', 'GsfSimTrackerHitG4StepLinks',
)
TRACKER_PASSTHROUGH = (
    'VXDCollection', 'ITKBarrelCollection', 'ITKEndcapCollection',
    'TPCCollection', 'OTKBarrelCollection', 'OTKEndcapCollection',
    'GsfG4MaterialSteps', 'GsfSimTrackerHitG4StepLinks',
    'GsfG4BremsPhotons', 'GsfG4BremsPhotonSteps',
    'VXDTrackerHits', 'ITKBarrelTrackerHits', 'ITKEndcapTrackerHits',
    'TPCTrackerHits', 'OTKBarrelTrackerHits', 'OTKEndcapTrackerHits',
    'VXDTrackerHitAssociation', 'ITKBarrelTrackerHitAssociation',
    'ITKEndcapTrackerHitAssociation', 'TPCTrackerHitAss',
    'OTKBarrelTrackerHitAssociation', 'OTKEndcapTrackerHitAssociation',
    'CompleteTracks', 'CompleteTracksParticleAssociation',
    'RecTofCollection', 'DndxTracks',
)
BP_CONTROLS = ('BP_INTERVALS', 'BP_INTERVAL_SELECTION_MODE', 'BP_LOSS_STATE_MODE', 'BP_TRUTH_OVERRIDE',
               'BP_DIFFUSE_AUGMENTED_RTS',
               'BP_ABSOLUTE_NEUTRAL_RTS', 'BP_NEUTRAL_THETA_MRAD', 'BP_NEUTRAL_PHI_MRAD',
               'BP_NEUTRAL_STOCHASTIC', 'BP_NEUTRAL_CONSTANT',
               'BP_MEAN_LOG_LOSS', 'BP_SIGMA_LOG_LOSS', 'BP_BACKWARD_SEED_SCALE',
               'BP_FREE_LOSS_FIT', 'BP_FREE_LOSS_MAX_LOG_LOSS', 'BP_FREE_LOSS_MAX_CALLS',
               'BP_FREE_LOSS_TOLERANCE',
               'BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE', 'BP_BEAM_SPOT_X', 'BP_BEAM_SPOT_Y',
               'BP_BEAM_SPOT_SIGMA_X', 'BP_BEAM_SPOT_SIGMA_Y',
               'BP_SEED_HIT_SELECTION',
               'BP_VERBOSE', 'BP_VERIFY_KF', 'BP_SELECTED')


def boolean(value):
    if value.lower() in ('1', 'true', 'yes', 'on'): return True
    if value.lower() in ('0', 'false', 'no', 'off'): return False
    raise ValueError('Expected a boolean, got ' + repr(value))


def integer(name, minimum):
    value = os.environ[name]
    if not re.fullmatch(r'[0-9]+', value) or int(value) < minimum:
        raise ValueError(name + ' must be an integer >= ' + str(minimum))
    return int(value)


def directory(repo, value):
    if not value.strip(): raise ValueError('Tuple paths must not be empty')
    path = Path(value).expanduser()
    return (path if path.is_absolute() else repo / path).resolve()


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError('Card template changed; expected exactly one ' + repr(old))
    return text.replace(old, new, 1)


def pass_through(text, reader, collections):
    """Extend only this worker's frozen card; shared GSF templates stay intact."""
    addition = (f'for _collection in {collections!r}:\n'
                f'    if _collection not in {reader}.collections:\n'
                f'        {reader}.collections.append(_collection)\n\n')
    anchor = ('# ApplicationMgr\n' if reader == 'podioinput'
              else 'from Configurables import ApplicationMgr\n')
    return replace_once(text, anchor, addition + anchor)


def create(path, text):
    with path.open('x') as stream: stream.write(text)


def prepare():
    repo = Path(os.environ['CEPCSW_BREAKPOINT_DIR']).resolve()
    source = directory(repo, os.environ['INPUT_TUPLEPATH'])
    output = directory(repo, os.environ['OUTPUT_TUPLEPATH'])
    if source == output: raise ValueError('Input and output directories must differ')
    requested = os.environ['STAGES'].split(',')
    if len(set(requested)) != len(requested) or any(s not in ORDER for s in requested):
        raise ValueError('STAGES must be a nonempty, nonduplicated subset of sim,trk,calodigi,rec,breakpoint')
    stages = [s for s in ORDER if s in requested]
    nevt, first, last = integer('NEVT', 1), integer('SEED_FIRST', 0), integer('SEED_LAST', 0)
    memory = integer('MEMORY_MB', 1)
    if first > last: raise ValueError('SEED_FIRST exceeds SEED_LAST')
    dry = boolean(os.environ['DRY_RUN'])
    sample_region = os.environ.get('SAMPLE_REGION', '').strip()
    if sample_region and sample_region not in ('barrel', 'endcap'):
        raise ValueError('SAMPLE_REGION must be barrel, endcap, or empty for legacy names')
    if sample_region and any(len(os.environ[key].split(',')) != 1
                             for key in ('PARTICLES', 'THETAS', 'TRANSVERSE_MOMENTA')):
        raise ValueError('Region-and-seed filenames require one particle and one legacy scan label per campaign')
    worker = repo / 'dump_breakpoint.sh'
    if not worker.is_file() or not os.access(worker, os.X_OK):
        raise ValueError('Worker missing/not executable: ' + str(worker))
    if not dry and not shutil.which('hep_sub'): raise ValueError('hep_sub is unavailable; use DRY_RUN=1 to prepare locally')

    # Refuse stale iteration requests instead of silently preparing a different fit.
    for retired in ('BP_MAX_ITERATIONS', 'BP_ITERATION_TOLERANCE'):
        if retired in os.environ:
            raise ValueError(retired + ' was removed; RecBreakpoint is one-pass only')
    if 'BP_LOSS_PRIOR_MODE' in os.environ:
        raise ValueError('BP_LOSS_PRIOR_MODE/LossPriorMode was removed; ordinary breakpoint fits use a Gaussian loss prior')
    # Freeze fit environment and card content now, not when a queued job starts.
    controls = {key: os.environ[key] for key in BP_CONTROLS if key in os.environ}
    for key in ('BP_TRUTH_OVERRIDE', 'BP_VERBOSE', 'BP_VERIFY_KF', 'BP_FREE_LOSS_FIT',
                'BP_DIFFUSE_AUGMENTED_RTS',
                'BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE'):
        if key in controls: controls[key] = str(int(boolean(controls[key])))
    templates = {}
    paths = {'sim': repo/'DumpGsfTrks/sim.py.bk', 'trk': repo/'DumpGsfTrks/trk.py.bk',
             'calodigi': repo/'DumpGsfTrks/calodigi.py.bk',
             'rec': repo/'DumpGsfTrks/rec.py.bk',
             'breakpoint': repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'}
    for stage in stages: templates[stage] = paths[stage].read_text()
    jobs = []
    skipped = 0
    for particle in os.environ['PARTICLES'].split(','):
        for theta in os.environ['THETAS'].split(','):
            for pt in os.environ['TRANSVERSE_MOMENTA'].split(','):
                for value in (particle, theta, pt):
                    if not re.fullmatch(r'[A-Za-z0-9.+-]+', value):
                        raise ValueError('Unsafe/empty sample label: ' + repr(value))
                for seed in range(first, last + 1):
                    sample = f'{sample_region}-{seed}' if sample_region else f'{particle}-{pt}-{theta}-{seed}'
                    carddir = output/'runcards'/sample
                    files = {s: str((output if s in stages else source)/f'{s}-{sample}.root')
                             for s in ('sim', 'trk', 'calodigi', 'rec')}
                    files['breakpoint'] = str(output/f'breakpoint_flat-{sample}.root')
                    missing_inputs = []
                    for stage, predecessor in [('trk','sim'), ('calodigi','trk'),
                                               ('rec','calodigi'), ('breakpoint','rec')]:
                        if stage in stages and predecessor not in stages:
                            path = Path(files[predecessor])
                            if not path.is_file() or path.stat().st_size == 0:
                                missing_inputs.append(str(path))
                    if missing_inputs:
                        skipped += 1
                        print(f'Skipping {sample}: missing or empty external input: '
                              + ', '.join(missing_inputs), flush=True)
                        continue
                    for path in [carddir, output/'outlog'/f'{sample}.out', output/'outlog'/f'{sample}.err'] + [Path(files[s]) for s in stages]:
                        if path.exists(): raise ValueError('Refusing to overwrite ' + str(path))
                    job = dict(repo=str(repo), sample=sample, sample_region=sample_region,
                               seed=seed, nevt=nevt,
                               stages=stages, files=files, cards={}, checksums={}, controls=controls, memory_mb=memory)
                    cards = {}
                    for stage in stages:
                        text = templates[stage]
                        if stage in ('sim', 'trk', 'calodigi', 'rec'):
                            text = replace_once(text, 'tuplepath = ""', 'tuplepath = ' + repr(str(output)))
                            text = replace_once(text, 'inputseed = 12340', f'inputseed = {seed}')
                            text = replace_once(text, 'evtmax = 12340', f'evtmax = {nevt}')
                            if stage == 'sim':
                                text = replace_once(text, "particlename = 'mu-'", 'particlename = ' + repr(particle))
                                if sample_region:
                                    text = replace_once(text, 'region = "barrel"', 'region = ' + repr(sample_region))
                                text = replace_once(text, '"sim_v01.root"', repr(files['sim']))
                            elif stage == 'trk':
                                text = replace_once(text, '"sim_v01.root"', repr(files['sim']))
                                text = replace_once(text, '"rec_v01.root"', repr(files['trk']))
                                text = replace_once(text, '"Digi_MUON.root"', repr(f'Digi_MUON-{sample}.root'))
                                # The tracker output is the calodigi input.
                                # Preserve simulated calorimeter hits and
                                # primary eBrem-photon provenance through it.
                                if 'calodigi' in stages:
                                    text = pass_through(text, 'podioinput',
                                                        SIM_CALO_COLLECTIONS + SIM_BREMS_COLLECTIONS)
                            elif stage == 'calodigi':
                                text = replace_once(text, 'digitizationseed = 12340', f'digitizationseed = {seed}')
                                text = replace_once(text, '"trk.root"', repr(files['trk']))
                                text = replace_once(text, '"calodigi.root"', repr(files['calodigi']))
                                text = replace_once(text, '"Digi_ECAL.root"', repr(f'Digi_ECAL-{sample}.root'))
                                text = replace_once(text, '"Digi_HCAL.root"', repr(f'Digi_HCAL-{sample}.root'))
                                # Keep tracker inputs and G4 provenance for rec and breakpoint.
                                text = pass_through(text, 'podioinput', TRACKER_PASSTHROUGH)
                            else:  # rec
                                text = replace_once(text, '"calodigi.root"', repr(files['calodigi']))
                                text = replace_once(text, '"rec.root"', repr(files['rec']))
                                text = replace_once(text, '"RecAnaTuple_TDR_o1_v01.root"',
                                                    repr(f'RecAnaTuple-{sample}.root'))
                                text = replace_once(text, '"Jets_TDR_o1_v01.root"',
                                                    repr(f'Jets-{sample}.root'))
                                text = pass_through(text, 'inp', TRACKER_PASSTHROUGH)
                        else:
                            env = dict(controls, BP_INPUT=files['rec'], BP_OUTPUT=files['breakpoint'], BP_EVENTS=str(nevt))
                            # Clear worker ambient BP_* so batch nodes cannot silently
                            # change physics or resurrect old selected-event controls.
                            text = ('import os\n'
                                    'for _key in list(os.environ):\n'
                                    '    if _key.startswith("BP_"): del os.environ[_key]\n'
                                    f'os.environ.update({env!r})\n\n' + text)
                        compile(text, str(carddir/f'{stage}.py'), 'exec')
                        cards[stage] = text
                        job['cards'][stage] = str(carddir/f'{stage}.py')
                        job['checksums'][stage] = hashlib.sha256(text.encode()).hexdigest()
                    jobs.append((carddir, job, cards))
    if not jobs:
        raise ValueError(f'No eligible jobs: skipped {skipped} samples with missing or empty external inputs')
    if len({str(d) for d, _, _ in jobs}) != len(jobs): raise ValueError('Duplicate sample labels')
    # Validate the entire campaign before creating files or submitting any job.
    (output/'outlog').mkdir(parents=True, exist_ok=True)
    for carddir, job, cards in jobs:
        carddir.mkdir(parents=True, exist_ok=False)
        for stage, text in cards.items(): create(Path(job['cards'][stage]), text)
        create(carddir/'job.json', json.dumps(job, indent=2)+'\n')
    print(f'Prepared {len(jobs)} jobs: {",".join(stages)}; output={output}', flush=True)
    print(f'Skipped {skipped} samples with missing or empty external inputs.', flush=True)
    print('Fit steering is frozen from run_breakpoint.py and explicit BP_* overrides.', flush=True)
    print('Interval selection follows the frozen card (Truth by default; Auto not implemented).', flush=True)
    print('Sample naming: ' + (sample_region + '-<seed>' if sample_region else 'legacy particle-pT-theta-seed'), flush=True)
    print('If simulation runs here, its gun energy stays in sim.py.bk and its theta range follows SAMPLE_REGION.', flush=True)
    submit_jobs([(carddir, job) for carddir, job, _ in jobs], dry)


def submit_jobs(jobs, dry):
    failures = 0
    for carddir, job in jobs:
        repo = Path(job['repo'])
        output = carddir.parent.parent
        worker = repo/'dump_breakpoint.sh'
        memory = job['memory_mb']
        command = ['hep_sub', str(worker), '-g', 'cms', '-mem', str(memory),
                   '-o', str(output/'outlog'/f'{job["sample"]}.out'),
                   '-e', str(output/'outlog'/f'{job["sample"]}.err'), '-argu', str(carddir/'job.json')]
        print(shlex.join(command), flush=True)
        if not dry:
            result = subprocess.run(command, cwd=repo, capture_output=True, text=True)
            print(result.stdout, end='', flush=True)
            print(result.stderr, end='', file=sys.stderr, flush=True)
            if result.returncode: failures += 1
            else:
                create(carddir/'submitted.json', json.dumps({'command':command, 'stdout':result.stdout,
                                                            'stderr':result.stderr},indent=2)+'\n')
    if failures: raise RuntimeError(f'{failures} submissions failed; manifests retained, inspect before resubmitting')


def submit_prepared(value):
    repo = Path(os.environ['CEPCSW_BREAKPOINT_DIR']).resolve()
    output = directory(repo, value)
    dry = boolean(os.environ['DRY_RUN'])
    sample_region = os.environ.get('SAMPLE_REGION', '').strip()
    if sample_region and sample_region not in ('barrel', 'endcap'):
        raise ValueError('SAMPLE_REGION must be barrel, endcap, or empty for legacy names')
    if not dry and not shutil.which('hep_sub'): raise ValueError('hep_sub is unavailable')
    jobs = []
    for manifest in sorted((output/'runcards').glob('*/job.json')):
        job = json.loads(manifest.read_text())
        # Historical momentum/theta cards may share this output directory.
        # Select only the campaign naming mode requested by this invocation.
        if job.get('sample_region', '') != sample_region: continue
        if Path(job['repo']) != repo: raise ValueError('Manifest belongs to another worktree')
        if (manifest.parent/'submitted.json').exists(): raise ValueError('Job already submitted: ' + str(manifest))
        if (manifest.parent/'started.json').exists(): raise ValueError('Job already started: ' + str(manifest))
        for stage in job['stages']:
            if Path(job['files'][stage]).exists(): raise ValueError('Output already exists: ' + job['files'][stage])
            if hashlib.sha256(Path(job['cards'][stage]).read_bytes()).hexdigest() != job['checksums'][stage]:
                raise ValueError('Frozen card changed: ' + job['cards'][stage])
        for suffix in ('out','err'):
            if (output/'outlog'/f'{job["sample"]}.{suffix}').exists():
                raise ValueError('Existing scheduler log: inspect job state before resubmission')
        jobs.append((manifest.parent,job))
    if not jobs: raise ValueError(f'No prepared {sample_region or "legacy"} job manifests under {output}')
    submit_jobs(jobs,dry)


def verify(path, stage):
    import ROOT
    file = ROOT.TFile.Open(str(path))
    if not file or file.IsZombie() or file.TestBit(ROOT.TFile.kRecovered):
        raise RuntimeError('Invalid ROOT output: ' + str(path))
    tree = file.Get('breakpoint' if stage == 'breakpoint' else 'events')
    if not tree or tree.GetEntries() == 0: raise RuntimeError('Empty output tree: ' + str(path))
    cleanup_ready = True
    if stage == 'calodigi':
        for name in BREAKPOINT_INPUT_COLLECTIONS + (
                'ECALBarrel', 'ECALEndcaps', 'HCALBarrel', 'HCALEndcaps',
                'GsfG4BremsPhotons', 'GsfG4BremsPhotonSteps'):
            if not tree.GetBranch(name): raise RuntimeError('Missing calodigi collection: ' + name)
        print(f'calodigi events={tree.GetEntries()}', flush=True)
    elif stage == 'rec':
        for name in BREAKPOINT_INPUT_COLLECTIONS + (
                'EcalCluster', 'CyberPFO', 'CyberPFOPID',
                'GsfG4BremsPhotons', 'GsfG4BremsPhotonSteps'):
            if not tree.GetBranch(name): raise RuntimeError('Missing rec collection: ' + name)
        print(f'rec events={tree.GetEntries()}', flush=True)
    elif stage == 'breakpoint':
        for name in ('rts_pt','backward_pt','truth_override_rts_pt','truth_override_backward_pt',
                     'truth_override_result_status', 'beam_guided_free_loss_rts_pt',
                     'beam_guided_free_loss_backward_pt', 'beam_guided_free_loss_result_status',
                     'beam_guided_free_loss_objective_nll2'):
            if not tree.GetBranch(name): raise RuntimeError('Missing flat branch: ' + name)
        for name in ('truth_pt', 'truth_match_status', 'truth_match_purity',
                     'charged_pfo_count', 'charged_ecal_cluster_count'):
            if not tree.GetBranch(name): raise RuntimeError('Missing matched flat branch: ' + name)
        neutral = file.Get('neutral_pfos')
        if not neutral or not all(neutral.GetBranch(name) for name in (
                'neutral_ecal_cluster_energy', 'ebrem_photon_ecal_entry_status',
                'ebrem_photon_last_step_status', 'ebrem_photon_last_step_process_subtype',
                'ebrem_photon_last_step_track_status', 'ebrem_photon_last_step_post_energy')):
            raise RuntimeError('Missing neutral-PFO event tree: ' + str(path))
        good = int(tree.GetEntries('status==1'))
        invalid_truth = int(tree.GetEntries('truth_override_result_status<0'))
        print(f'Flat rows={tree.GetEntries()}, ordinary success={good}, invalid oracle={invalid_truth}', flush=True)
        # Job completion/output integrity, not per-track fit success, permits
        # cleanup. Failed ordinary/oracle rows remain tagged in the flat tuple.
    else: print(f'{stage} events={tree.GetEntries()}', flush=True)
    file.Close()
    return cleanup_ready


def file_identity(path):
    stat = path.stat()
    return stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns


def cleanup_intermediate(job, manifest, stage, produced_identity, cleanup_ready):
    """Delete only this job's unchanged intermediate after its consumer verifies."""
    consumer = {'trk': 'calodigi', 'calodigi': 'rec'}[stage]
    if stage not in job['stages']:
        return {'status':'retained_external'}
    if consumer not in job['stages']:
        return {'status':'retained_no_downstream'}
    if not cleanup_ready:
        print(f'Retaining {stage} output: {consumer} output has not passed verification.', flush=True)
        return {'status':'retained_unverified_output'}
    output_dir = manifest.parent.parent.parent
    expected = output_dir / f'{stage}-{job["sample"]}.root'
    intermediate = Path(job['files'][stage])
    if intermediate != expected or intermediate.is_symlink() or not intermediate.is_file():
        raise RuntimeError('Refusing cleanup of unexpected/nonregular intermediate: ' + str(intermediate))
    if produced_identity is None or file_identity(intermediate) != produced_identity:
        raise RuntimeError('Intermediate changed since production; refusing cleanup: ' + str(intermediate))
    intermediate.unlink()
    print('Removed verified intermediate ' + stage + ' output: ' + str(intermediate), flush=True)
    return {'status':'removed', 'path':str(intermediate)}


def run(manifest):
    job = json.loads(manifest.read_text())
    repo = Path(job['repo'])
    if Path.cwd().resolve() != repo: raise ValueError('Manifest belongs to a different worktree')
    runner = repo/'build.105.0.0.x86_64-el9-gcc11-opt/run'
    if not runner.is_file(): raise ValueError('Configured EL9 build runner is missing')
    for stage in job['stages']:
        card = Path(job['cards'][stage])
        if hashlib.sha256(card.read_bytes()).hexdigest() != job['checksums'][stage]:
            raise ValueError('Frozen card changed: ' + str(card))
        if Path(job['files'][stage]).exists(): raise ValueError('Output already exists: ' + job['files'][stage])
    # Exclusive marker prevents two workers using the same outputs concurrently.
    create(manifest.parent/'started.json', json.dumps({'pid':os.getpid(), 'host':os.uname().nodename})+'\n')
    produced = {}
    cleanup = {stage: {'status': 'retained_external' if stage not in job['stages']
                       else 'retained_no_downstream'} for stage in ('trk', 'calodigi')}
    for stage in job['stages']:
        print('Running ' + stage + ': ' + job['cards'][stage], flush=True)
        subprocess.run([str(runner), 'gaudirun.py', job['cards'][stage]], cwd=repo, check=True)
        ready = verify(Path(job['files'][stage]), stage)
        if stage in ('trk', 'calodigi'):
            produced[stage] = file_identity(Path(job['files'][stage]))
        if stage == 'calodigi':
            cleanup['trk'] = cleanup_intermediate(job, manifest, 'trk', produced.get('trk'), ready)
        elif stage == 'rec':
            cleanup['calodigi'] = cleanup_intermediate(job, manifest, 'calodigi',
                                                        produced.get('calodigi'), ready)
    create(manifest.parent/'completed.json', json.dumps({'outputs':job['files'], 'stages':job['stages'],
                                                        'intermediate_cleanup':cleanup}, indent=2)+'\n')
    print('Completed. Simulation, rec, and external inputs retained; no GSF workflow was run.', flush=True)


if __name__ == '__main__':
    try:
        if sys.argv[1:] == ['prepare']: prepare()
        elif len(sys.argv) == 3 and sys.argv[1] == 'submit': submit_prepared(sys.argv[2])
        elif len(sys.argv) == 3 and sys.argv[1] == 'run': run(Path(sys.argv[2]).resolve())
        else: raise ValueError('Usage: batch_breakpoint.py prepare | submit OUTPUT_DIR | run JOB.json')
    except Exception as error:
        print('Breakpoint batch error: ' + str(error), file=sys.stderr)
        sys.exit(1)
