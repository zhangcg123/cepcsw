"""Standard-library tests for independent batch planning; no Gaudi/Condor jobs."""
import contextlib
import importlib.util
import io
import itertools
import json
import os
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('batch', REPO/'Reconstruction/RecBreakpoint/options/batch_breakpoint.py')
batch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(batch)


class BatchTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='breakpoint_batch_test_')
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name)
        for rel in ['DumpGsfTrks/sim.py.bk', 'DumpGsfTrks/trk.py.bk',
                    'Reconstruction/RecBreakpoint/options/run_breakpoint.py', 'dump_breakpoint.sh']:
            dest = self.repo/rel; dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(REPO/rel, dest)
        (self.repo/'inputs').mkdir()
        for stage in ['sim','trk']:
            (self.repo/'inputs'/f'{stage}-e--2.0-85-12.root').write_bytes(b'planning fixture')
        self.env = dict(CEPCSW_BREAKPOINT_DIR=str(self.repo), INPUT_TUPLEPATH='inputs', OUTPUT_TUPLEPATH='outputs',
                        STAGES='trk,breakpoint', NEVT='2', SEED_FIRST='12', SEED_LAST='12', MEMORY_MB='5000',
                        DRY_RUN='1', PARTICLES='e-', THETAS='85', TRANSVERSE_MOMENTA='2.0')

    def prepare(self, **overrides):
        with patch.dict(os.environ, dict(self.env, **overrides), clear=True), contextlib.redirect_stdout(io.StringIO()):
            batch.prepare()

    def manifest(self, output='outputs'):
        return json.loads((self.repo/output/'runcards/e--2.0-85-12/job.json').read_text())

    def test_all_stage_subsets(self):
        for n in (1,2,3):
            for selected in itertools.combinations(batch.ORDER,n):
                out = '_'.join(selected)
                self.prepare(STAGES=','.join(reversed(selected)), OUTPUT_TUPLEPATH=out)
                job = self.manifest(out)
                self.assertEqual(job['stages'],list(selected))
                for stage in ('sim','trk'):
                    self.assertEqual(Path(job['files'][stage]).parent,self.repo/(out if stage in selected else 'inputs'))
                if 'breakpoint' in selected:
                    text=Path(job['cards']['breakpoint']).read_text()
                    self.assertIn(repr(job['files']['trk']),text)
                    self.assertNotIn('RecGsfTracking(',text)

    def test_controls_frozen_and_originals_unchanged(self):
        source = self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        before = source.read_bytes()
        self.prepare(BP_TRUTH_OVERRIDE='true', BP_INTERVAL_SELECTION_MODE='Manual', BP_INTERVALS='5', BP_BACKWARD_SEED_SCALE='100')
        job = self.manifest()
        self.assertEqual(job['controls']['BP_TRUTH_OVERRIDE'],'1')
        self.assertEqual(job['controls']['BP_INTERVALS'],'5')
        self.assertEqual(job['controls']['BP_INTERVAL_SELECTION_MODE'],'Manual')
        self.assertEqual(source.read_bytes(),before)
        for stage, card in job['cards'].items():
            self.assertEqual(batch.hashlib.sha256(Path(card).read_bytes()).hexdigest(),job['checksums'][stage])
        with self.assertRaises(ValueError): self.prepare()

    def test_invalid_campaigns_leave_no_output(self):
        for override in [dict(STAGES=''),dict(STAGES='trk,trk'),dict(STAGES='gsf'),dict(SEED_LAST='11'),
                         dict(NEVT='0'),dict(MEMORY_MB='bad'),dict(PARTICLES='../escape'),
                         dict(INPUT_TUPLEPATH='outputs'),dict(BP_TRUTH_OVERRIDE='maybe')]:
            with self.subTest(override=override),self.assertRaises(ValueError): self.prepare(**override)
            self.assertFalse((self.repo/'outputs').exists())

    def test_missing_and_empty_inputs_skip_but_later_seed_submits(self):
        # Seed11 missing, seed12 valid, seed13 empty, seed14 valid.
        (self.repo/'inputs/sim-e--2.0-85-13.root').write_bytes(b'')
        (self.repo/'inputs/sim-e--2.0-85-14.root').write_bytes(b'planning fixture')
        env = dict(self.env, SEED_FIRST='11', SEED_LAST='14', DRY_RUN='0')
        output = io.StringIO()
        with patch.dict(os.environ, env, clear=True), contextlib.redirect_stdout(output), \
             patch.object(batch.shutil, 'which', return_value='/mock/hep_sub'), \
             patch.object(batch.subprocess, 'run') as run:
            run.return_value.returncode = 0
            run.return_value.stdout = 'Submitted mock job\n'; run.return_value.stderr = ''
            batch.prepare()
        self.assertEqual(run.call_count, 2)
        for seed in (12, 14):
            self.assertTrue((self.repo/f'outputs/runcards/e--2.0-85-{seed}/job.json').is_file())
        for seed in (11, 13):
            self.assertFalse((self.repo/f'outputs/runcards/e--2.0-85-{seed}').exists())
            self.assertIn(f'Skipping e--2.0-85-{seed}', output.getvalue())
        self.assertIn('Skipped 2 samples', output.getvalue())

    def test_all_missing_inputs_leave_no_output(self):
        with self.assertRaisesRegex(ValueError, 'No eligible jobs: skipped 2'):
            self.prepare(SEED_FIRST='13', SEED_LAST='14')
        self.assertFalse((self.repo/'outputs').exists())

    def test_breakpoint_only_skips_missing_tracker(self):
        self.prepare(STAGES='breakpoint', SEED_LAST='13')
        self.assertEqual(self.manifest()['stages'], ['breakpoint'])
        self.assertFalse((self.repo/'outputs/runcards/e--2.0-85-13').exists())

    def test_generated_sim_needs_no_external_sim(self):
        self.prepare(STAGES='sim,trk,breakpoint', SEED_FIRST='13', SEED_LAST='13')
        self.assertTrue((self.repo/'outputs/runcards/e--2.0-85-13/job.json').is_file())

    def test_template_drift_fails_before_submission(self):
        path=self.repo/'DumpGsfTrks/trk.py.bk'
        path.write_text(path.read_text().replace('evtmax = 12340','evtmax = 2'))
        with self.assertRaises(ValueError): self.prepare()
        self.assertFalse((self.repo/'outputs').exists())

    def test_existing_root_refused(self):
        (self.repo/'outputs').mkdir()
        path=self.repo/'outputs/trk-e--2.0-85-12.root'; path.write_bytes(b'preserve')
        with self.assertRaises(ValueError): self.prepare()
        self.assertEqual(path.read_bytes(),b'preserve')

    def test_submission_arguments_without_real_scheduler(self):
        with patch.object(batch.shutil,'which',return_value='/mock/hep_sub'), patch.object(batch.subprocess,'run') as run:
            run.return_value.returncode=0
            run.return_value.stdout='Submitted mock job\n'; run.return_value.stderr=''
            self.prepare(DRY_RUN='0')
        args=run.call_args.args[0]
        self.assertEqual(args[:2],['hep_sub',str(self.repo/'dump_breakpoint.sh')])
        self.assertEqual(args[args.index('-mem')+1],'5000')
        self.assertEqual(args[-2],'-argu')
        self.assertTrue(Path(args[-1]).is_file())

    def test_submit_prepared_preserves_cards_and_blocks_duplicates(self):
        self.prepare()
        job=self.manifest()
        hashes=dict(job['checksums'])
        with patch.dict(os.environ,self.env,clear=True), contextlib.redirect_stdout(io.StringIO()):
            batch.submit_prepared('outputs')  # dry run: no scheduler
        self.assertEqual(self.manifest()['checksums'],hashes)
        marker=Path(job['cards']['trk']).parent/'submitted.json'; marker.write_text('{}')
        with patch.dict(os.environ,self.env,clear=True), self.assertRaises(ValueError):
            batch.submit_prepared('outputs')

    def cleanup_fixture(self, stages='trk,breakpoint'):
        self.prepare(STAGES=stages)
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        tracker=Path(job['files']['trk'])
        if 'trk' in job['stages']: tracker.write_bytes(b'job-owned tracker')
        return job,manifest,tracker,batch.file_identity(tracker)

    def test_cleanup_only_owned_complete_tracker(self):
        job,manifest,tracker,identity=self.cleanup_fixture()
        result=batch.cleanup_tracker(job,manifest,identity,True)
        self.assertEqual(result['status'],'removed');self.assertFalse(tracker.exists())
        self.assertTrue(Path(job['files']['sim']).exists())

    def test_cleanup_retains_failed_refits(self):
        job,manifest,tracker,identity=self.cleanup_fixture()
        result=batch.cleanup_tracker(job,manifest,identity,False)
        self.assertEqual(result['status'],'retained_incomplete_refits');self.assertTrue(tracker.exists())

    def test_cleanup_retains_external_input(self):
        job,manifest,tracker,identity=self.cleanup_fixture('breakpoint')
        result=batch.cleanup_tracker(job,manifest,None,True)
        self.assertEqual(result['status'],'retained_external');self.assertTrue(tracker.exists())

    def test_cleanup_retains_trk_only_output(self):
        job,manifest,tracker,identity=self.cleanup_fixture('trk')
        result=batch.cleanup_tracker(job,manifest,identity,True)
        self.assertEqual(result['status'],'retained_no_downstream');self.assertTrue(tracker.exists())

    def test_cleanup_refuses_changed_or_redirected_tracker(self):
        job,manifest,tracker,identity=self.cleanup_fixture()
        tracker.write_bytes(b'changed after production')
        with self.assertRaises(RuntimeError):batch.cleanup_tracker(job,manifest,identity,True)
        tracker.unlink()
        shared=self.repo/'inputs/trk-e--2.0-85-12.root'
        tracker.symlink_to(shared)
        with self.assertRaises(RuntimeError):batch.cleanup_tracker(job,manifest,batch.file_identity(tracker),True)
        self.assertTrue(shared.exists())

    def test_worker_failure_does_not_cleanup_tracker(self):
        self.prepare()
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        runner=self.repo/'build.105.0.0.x86_64-el9-gcc11-opt/run'
        runner.parent.mkdir(parents=True);runner.write_text('fixture')
        def execute(command, **kwargs):
            if Path(command[-1]).name=='trk.py':
                Path(job['files']['trk']).write_bytes(b'produced tracker')
            else:
                raise batch.subprocess.CalledProcessError(1,command)
        with patch.object(Path,'cwd',return_value=self.repo), patch.object(batch.subprocess,'run',side_effect=execute), \
             patch.object(batch,'verify',return_value=False), patch.object(batch,'cleanup_tracker') as cleanup:
            with self.assertRaises(batch.subprocess.CalledProcessError):batch.run(manifest)
            cleanup.assert_not_called()
        self.assertTrue(Path(job['files']['trk']).exists())
        self.assertFalse((manifest.parent/'completed.json').exists())


if __name__ == '__main__': unittest.main()
