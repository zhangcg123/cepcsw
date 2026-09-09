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

ORDER = ('sim', 'trk', 'breakpoint')
BP_CONTROLS = ('BP_INTERVALS', 'BP_INTERVAL_SELECTION_MODE', 'BP_LOSS_STATE_MODE', 'BP_TRUTH_OVERRIDE',
               'BP_MEAN_LOG_LOSS', 'BP_SIGMA_LOG_LOSS', 'BP_BACKWARD_SEED_SCALE',
               'BP_SEED_HIT_SELECTION', 'BP_MAX_ITERATIONS', 'BP_ITERATION_TOLERANCE',
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


def create(path, text):
    with path.open('x') as stream: stream.write(text)


def prepare():
    repo = Path(os.environ['CEPCSW_BREAKPOINT_DIR']).resolve()
    source = directory(repo, os.environ['INPUT_TUPLEPATH'])
    output = directory(repo, os.environ['OUTPUT_TUPLEPATH'])
    if source == output: raise ValueError('Input and output directories must differ')
    requested = os.environ['STAGES'].split(',')
    if len(set(requested)) != len(requested) or any(s not in ORDER for s in requested):
        raise ValueError('STAGES must be a nonempty, nonduplicated subset of sim,trk,breakpoint')
    stages = [s for s in ORDER if s in requested]
    nevt, first, last = integer('NEVT', 1), integer('SEED_FIRST', 0), integer('SEED_LAST', 0)
    memory = integer('MEMORY_MB', 1)
    if first > last: raise ValueError('SEED_FIRST exceeds SEED_LAST')
    dry = boolean(os.environ['DRY_RUN'])
    worker = repo / 'dump_breakpoint.sh'
    if not worker.is_file() or not os.access(worker, os.X_OK):
        raise ValueError('Worker missing/not executable: ' + str(worker))
    if not dry and not shutil.which('hep_sub'): raise ValueError('hep_sub is unavailable; use DRY_RUN=1 to prepare locally')

    # Freeze fit environment and card content now, not when a queued job starts.
    controls = {key: os.environ[key] for key in BP_CONTROLS if key in os.environ}
    for key in ('BP_TRUTH_OVERRIDE', 'BP_VERBOSE', 'BP_VERIFY_KF'):
        if key in controls: controls[key] = str(int(boolean(controls[key])))
    templates = {}
    paths = {'sim': repo/'DumpGsfTrks/sim.py.bk', 'trk': repo/'DumpGsfTrks/trk.py.bk',
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
                    sample = f'{particle}-{pt}-{theta}-{seed}'
                    carddir = output/'runcards'/sample
                    files = {s: str((output if s in stages else source)/f'{s}-{sample}.root') for s in ('sim','trk')}
                    files['breakpoint'] = str(output/f'breakpoint_flat-{sample}.root')
                    missing_inputs = []
                    for stage, predecessor in [('trk','sim'), ('breakpoint','trk')]:
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
                    job = dict(repo=str(repo), sample=sample, seed=seed, nevt=nevt,
                               stages=stages, files=files, cards={}, checksums={}, controls=controls, memory_mb=memory)
                    cards = {}
                    for stage in stages:
                        text = templates[stage]
                        if stage in ('sim','trk'):
                            text = replace_once(text, 'tuplepath = ""', 'tuplepath = ' + repr(str(output)))
                            text = replace_once(text, 'inputseed = 12340', f'inputseed = {seed}')
                            text = replace_once(text, 'evtmax = 12340', f'evtmax = {nevt}')
                            if stage == 'sim':
                                text = replace_once(text, "particlename = 'mu-'", 'particlename = ' + repr(particle))
                                text = replace_once(text, '"sim_v01.root"', repr(files['sim']))
                            else:
                                text = replace_once(text, '"sim_v01.root"', repr(files['sim']))
                                text = replace_once(text, '"rec_v01.root"', repr(files['trk']))
                                text = replace_once(text, '"Digi_MUON.root"', repr(f'Digi_MUON-{sample}.root'))
                        else:
                            env = dict(controls, BP_INPUT=files['trk'], BP_OUTPUT=files['breakpoint'], BP_EVENTS=str(nevt))
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
    print('Simulation momentum/theta ranges remain those in sim.py.bk.', flush=True)
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
    if not dry and not shutil.which('hep_sub'): raise ValueError('hep_sub is unavailable')
    jobs = []
    for manifest in sorted((output/'runcards').glob('*/job.json')):
        job = json.loads(manifest.read_text())
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
    if not jobs: raise ValueError('No prepared job manifests under ' + str(output))
    submit_jobs(jobs,dry)


def verify(path, stage):
    import ROOT
    file = ROOT.TFile.Open(str(path))
    if not file or file.IsZombie() or file.TestBit(ROOT.TFile.kRecovered):
        raise RuntimeError('Invalid ROOT output: ' + str(path))
    tree = file.Get('breakpoint' if stage == 'breakpoint' else 'events')
    if not tree or tree.GetEntries() == 0: raise RuntimeError('Empty output tree: ' + str(path))
    cleanup_ready = False
    if stage == 'breakpoint':
        for name in ('rts_pt','backward_pt','truth_override_rts_pt','truth_override_backward_pt','truth_override_result_status'):
            if not tree.GetBranch(name): raise RuntimeError('Missing flat branch: ' + name)
        good = int(tree.GetEntries('status==1'))
        invalid_truth = int(tree.GetEntries('truth_override_result_status<0'))
        print(f'Flat rows={tree.GetEntries()}, ordinary success={good}, invalid oracle={invalid_truth}', flush=True)
        # Job completion/output integrity, not per-track fit success, permits
        # cleanup. Failed ordinary/oracle rows remain tagged in the flat tuple.
        cleanup_ready = True
    else: print(f'{stage} events={tree.GetEntries()}', flush=True)
    file.Close()
    return cleanup_ready


def file_identity(path):
    stat = path.stat()
    return stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns


def cleanup_tracker(job, manifest, produced_identity, cleanup_ready):
    """Delete only this job's own, unchanged intermediate after verified output."""
    if 'trk' not in job['stages']:
        return {'status':'retained_external'}
    if 'breakpoint' not in job['stages']:
        return {'status':'retained_no_downstream'}
    if not cleanup_ready:
        print('Retaining tracker output: breakpoint output has not passed verification.', flush=True)
        return {'status':'retained_unverified_output'}
    output_dir = manifest.parent.parent.parent
    expected = output_dir / f'trk-{job["sample"]}.root'
    tracker = Path(job['files']['trk'])
    if tracker != expected or tracker.is_symlink() or not tracker.is_file():
        raise RuntimeError('Refusing cleanup of unexpected/nonregular tracker path: ' + str(tracker))
    if produced_identity is None or file_identity(tracker) != produced_identity:
        raise RuntimeError('Tracker output changed since production; refusing cleanup: ' + str(tracker))
    tracker.unlink()
    print('Removed verified intermediate tracker output: ' + str(tracker), flush=True)
    return {'status':'removed', 'path':str(tracker)}


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
    produced_tracker = None
    cleanup_ready = False
    for stage in job['stages']:
        print('Running ' + stage + ': ' + job['cards'][stage], flush=True)
        subprocess.run([str(runner), 'gaudirun.py', job['cards'][stage]], cwd=repo, check=True)
        ready = verify(Path(job['files'][stage]), stage)
        if stage == 'trk': produced_tracker = file_identity(Path(job['files']['trk']))
        if stage == 'breakpoint': cleanup_ready = ready
    cleanup = cleanup_tracker(job, manifest, produced_tracker, cleanup_ready)
    create(manifest.parent/'completed.json', json.dumps({'outputs':job['files'], 'stages':job['stages'],
                                                        'tracker_cleanup':cleanup}, indent=2)+'\n')
    print('Completed. Simulation/external inputs retained; no GSF workflow was run.', flush=True)


if __name__ == '__main__':
    try:
        if sys.argv[1:] == ['prepare']: prepare()
        elif len(sys.argv) == 3 and sys.argv[1] == 'submit': submit_prepared(sys.argv[2])
        elif len(sys.argv) == 3 and sys.argv[1] == 'run': run(Path(sys.argv[2]).resolve())
        else: raise ValueError('Usage: batch_breakpoint.py prepare | submit OUTPUT_DIR | run JOB.json')
    except Exception as error:
        print('Breakpoint batch error: ' + str(error), file=sys.stderr)
        sys.exit(1)
