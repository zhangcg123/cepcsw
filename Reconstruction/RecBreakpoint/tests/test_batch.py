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
import types
import unittest
from unittest.mock import MagicMock, patch

REPO = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('batch', REPO/'Reconstruction/RecBreakpoint/options/batch_breakpoint.py')
batch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(batch)


class BatchTest(unittest.TestCase):
    def test_retired_loss_prior_control_rejected_before_preparing_jobs(self):
        self.assertNotIn('BP_LOSS_PRIOR_MODE', batch.BP_CONTROLS)
        for value in ('Unconstrained', 'Fixed', 'Gaussian'):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, 'LossPriorMode was removed'):
                self.prepare(BP_LOSS_PRIOR_MODE=value)
            self.assertFalse((self.repo/'outputs').exists())

    def test_standalone_card_rejects_retired_loss_prior_before_gaudi(self):
        import runpy
        card = self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        for value in ('Unconstrained', 'Fixed', 'Gaussian'):
            with self.subTest(value=value), patch.dict(os.environ, {'BP_LOSS_PRIOR_MODE': value}, clear=True):
                with self.assertRaisesRegex(ValueError, 'LossPriorMode was removed'):
                    runpy.run_path(str(card))

    def test_gaussian_prior_and_state_representations_remain(self):
        self.prepare()
        card = Path(self.manifest()['cards']['breakpoint']).read_text()
        assignments = '\n'.join(line for line in card.splitlines()
                                if line.startswith(('fit.MeanLogLoss =', 'fit.SigmaLogLoss =', 'fit.LossStateMode =')))
        for mode in ('LocalMarginal', 'Persistent6D'):
            fit = types.SimpleNamespace()
            with self.subTest(mode=mode), patch.dict(os.environ, {'BP_LOSS_STATE_MODE': mode}, clear=True):
                exec(assignments, {'fit':fit, 'os':os})
            self.assertEqual(fit.LossStateMode, mode)
            self.assertEqual(fit.MeanLogLoss, 0)
            self.assertEqual(fit.SigmaLogLoss, .001)
        self.assertNotIn('fit.LossPriorMode', card)

    def test_card_explicitly_steers_every_algorithm_property(self):
        import ast
        import re
        package = REPO/'Reconstruction/RecBreakpoint'
        header = (package/'src/RecBreakpoint.h').read_text()
        source = (package/'src/RecBreakpoint.cpp').read_text()
        properties = set(re.findall(r'Gaudi::Property<[^;]+?\{this, "([^"]+)"', header))
        properties.update(re.findall(r'declareProperty\("([^"]+)"', source))
        card = ast.parse((package/'options/run_breakpoint.py').read_text())
        explicit = {target.attr for node in ast.walk(card) if isinstance(node, ast.Assign)
                    for target in node.targets if isinstance(target, ast.Attribute)
                    and isinstance(target.value, ast.Name) and target.value.id == 'fit'}
        self.assertGreater(len(properties), 25)
        self.assertEqual(properties - explicit, set())

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='breakpoint_batch_test_')
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name)
        for rel in ['DumpGsfTrks/sim.py.bk', 'DumpGsfTrks/trk.py.bk',
                    'Reconstruction/RecBreakpoint/options/run_breakpoint.py', 'dump_breakpoint.sh',
                    'subbreakpointjobs.sh', 'Reconstruction/RecBreakpoint/options/batch_breakpoint.py']:
            dest = self.repo/rel; dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(REPO/rel, dest)
        (self.repo/'inputs').mkdir()
        for stage in ['sim','trk']:
            (self.repo/'inputs'/f'{stage}-e--2.0-85-12.root').write_bytes(b'planning fixture')
        (self.repo/'inputs/sim-barrel-12.root').write_bytes(b'planning fixture')
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

    def test_backward_seed_default_and_explicit_override(self):
        for output, override, expected in [('seed_default', {}, 100.0),
                                           ('seed_explicit', {'BP_BACKWARD_SEED_SCALE':'1'}, 1.0)]:
            self.prepare(OUTPUT_TUPLEPATH=output, **override)
            job = self.manifest(output)
            card = Path(job['cards']['breakpoint']).read_text()
            assignment = next(line for line in card.splitlines()
                              if line.startswith('fit.BackwardSeedScale ='))
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, job['controls'], clear=True):
                exec(assignment, {'fit':fit, 'os':os})
            self.assertEqual(fit.BackwardSeedScale, expected)

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

    def test_free_loss_controls_and_default(self):
        self.prepare(OUTPUT_TUPLEPATH='free_default')
        default = self.manifest('free_default')
        card = Path(default['cards']['breakpoint']).read_text()
        assignments = '\n'.join(line for line in card.splitlines() if line.startswith('fit.FreeLoss'))
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, {}, clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertTrue(fit.FreeLossFit)
        self.assertEqual(fit.FreeLossMaxLogLoss, 1)
        self.assertEqual(fit.FreeLossMaxCallsPerStart, 180)
        self.assertEqual(fit.FreeLossTolerance, .001)
        self.prepare(OUTPUT_TUPLEPATH='free_enabled', BP_FREE_LOSS_FIT='true',
                     BP_FREE_LOSS_MAX_LOG_LOSS='.5',
                     BP_FREE_LOSS_MAX_CALLS='100', BP_FREE_LOSS_TOLERANCE='.002')
        job = self.manifest('free_enabled')
        self.assertEqual(job['controls']['BP_FREE_LOSS_FIT'], '1')
        with patch.dict(os.environ, job['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertTrue(fit.FreeLossFit)
        self.assertEqual(fit.FreeLossMaxLogLoss, .5)
        self.assertEqual(fit.FreeLossMaxCallsPerStart, 100)
        self.assertEqual(fit.FreeLossTolerance, .002)
        self.prepare(OUTPUT_TUPLEPATH='free_disabled', BP_FREE_LOSS_FIT='false')
        disabled = self.manifest('free_disabled')
        self.assertEqual(disabled['controls']['BP_FREE_LOSS_FIT'], '0')
        with patch.dict(os.environ, disabled['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertFalse(fit.FreeLossFit)
        with self.assertRaises(ValueError):
            self.prepare(OUTPUT_TUPLEPATH='free_invalid', BP_FREE_LOSS_FIT='maybe')

    def test_free_loss_shares_sigma_without_a_second_width_control(self):
        self.assertIn('BP_SIGMA_LOG_LOSS', batch.BP_CONTROLS)
        self.assertFalse(any('SIGMA_LOG_LOSS' in name and name != 'BP_SIGMA_LOG_LOSS'
                             for name in batch.BP_CONTROLS))
        for sigma in ('0.001', '0.05'):
            output='shared_sigma_'+sigma
            self.prepare(OUTPUT_TUPLEPATH=output, BP_FREE_LOSS_FIT='true', BP_SIGMA_LOG_LOSS=sigma)
            job=self.manifest(output)
            assignments='\n'.join(line for line in Path(job['cards']['breakpoint']).read_text().splitlines()
                                  if line.startswith(('fit.SigmaLogLoss =', 'fit.FreeLossFit =')))
            fit=types.SimpleNamespace()
            with patch.dict(os.environ,job['controls'],clear=True):
                exec(assignments,{'fit':fit,'os':os})
            self.assertTrue(fit.FreeLossFit)
            self.assertEqual(fit.SigmaLogLoss,float(sigma))

    def test_beam_guided_free_loss_is_parallel_and_default_on(self):
        self.prepare(OUTPUT_TUPLEPATH='beam_default')
        job = self.manifest('beam_default')
        card = Path(job['cards']['breakpoint']).read_text()
        selected = ('fit.FreeLossBeamSpotObjective =', 'fit.BeamSpotX =', 'fit.BeamSpotY =',
                    'fit.BeamSpotSigmaX =', 'fit.BeamSpotSigmaY =')
        assignments = '\n'.join(line for line in card.splitlines() if line.startswith(selected))
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, job['controls'], clear=True):
            exec(assignments, {'fit': fit, 'os': os})
        self.assertTrue(fit.FreeLossBeamSpotObjective)
        self.assertEqual((fit.BeamSpotX, fit.BeamSpotY), (0, 0))
        self.assertEqual((fit.BeamSpotSigmaX, fit.BeamSpotSigmaY), (.0145, 3.6e-5))
        self.assertIn('OutputTracksBeamGuidedFreeLossRTS', card)
        self.assertIn('OutputTracksBeamGuidedFreeLossBackwardFilter', card)
        self.prepare(OUTPUT_TUPLEPATH='beam_off', BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE='false')
        off = self.manifest('beam_off')
        self.assertEqual(off['controls']['BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE'], '0')
        with patch.dict(os.environ, off['controls'], clear=True):
            exec(assignments, {'fit': fit, 'os': os})
        self.assertFalse(fit.FreeLossBeamSpotObjective)

    def test_submission_shell_freezes_shared_loss_sigma(self):
        # This fixture checks plumbing, independent of the user's active campaign default.
        script = self.repo/'subbreakpointjobs.sh'
        text = script.read_text()
        import re
        text, count = re.subn(r'(BP_SIGMA_LOG_LOSS:-)[^}]+', lambda match: match[1]+'0.05', text)
        self.assertEqual(count, 1)
        script.write_text(text)
        for output, override, expected in [('sigma_default', None, '0.05'), ('sigma_override', '0.01', '0.01')]:
            env = dict(self.env, OUTPUT_TUPLEPATH=output)
            if override is not None: env['BP_SIGMA_LOG_LOSS'] = override
            result = batch.subprocess.run(['bash', str(self.repo/'subbreakpointjobs.sh')],
                                          env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            job = json.loads((self.repo/output/'runcards/barrel-12/job.json').read_text())
            self.assertEqual(job['controls']['BP_SIGMA_LOG_LOSS'], expected)
            card = Path(job['cards']['breakpoint']).read_text()
            self.assertIn(repr('BP_SIGMA_LOG_LOSS')+': '+repr(expected), card)
            self.assertIn('fit.SigmaLogLoss = float(os.environ.get("BP_SIGMA_LOG_LOSS", "0.001"))', card)

    def test_retired_iteration_controls_are_rejected(self):
        for name in ('BP_MAX_ITERATIONS', 'BP_ITERATION_TOLERANCE'):
            self.assertNotIn(name, batch.BP_CONTROLS)
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'removed'):
                self.prepare(**{name:'1'})
            self.assertFalse((self.repo/'outputs').exists())

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
        self.assertEqual(args[args.index('-g')+1],'cms')
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

    def test_cleanup_retains_unverified_output(self):
        job,manifest,tracker,identity=self.cleanup_fixture()
        result=batch.cleanup_tracker(job,manifest,identity,False)
        self.assertEqual(result['status'],'retained_unverified_output');self.assertTrue(tracker.exists())

    def test_bad_fit_rows_do_not_block_cleanup(self):
        for good in (0, 1, 2):
            with self.subTest(ordinary_success=good):
                tree = MagicMock()
                tree.GetEntries.side_effect = lambda cut=None: {
                    None: 2, 'status==1': good, 'truth_override_result_status<0': 1}[cut]
                file = MagicMock()
                file.IsZombie.return_value = False
                file.TestBit.return_value = False
                file.Get.return_value = tree
                root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path: file, kRecovered=1))
                with patch.dict('sys.modules', ROOT=root), contextlib.redirect_stdout(io.StringIO()):
                    self.assertTrue(batch.verify(Path('fixture.root'), 'breakpoint'))
                file.Close.assert_called_once()

    def test_invalid_flat_output_still_blocks_cleanup(self):
        for defect in ('zombie', 'recovered', 'empty', 'missing_branch'):
            with self.subTest(defect=defect):
                tree = MagicMock()
                tree.GetEntries.return_value = 0 if defect == 'empty' else 2
                tree.GetBranch.return_value = None if defect == 'missing_branch' else object()
                file = MagicMock()
                file.IsZombie.return_value = defect == 'zombie'
                file.TestBit.return_value = defect == 'recovered'
                file.Get.return_value = tree
                root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path: file, kRecovered=1))
                with patch.dict('sys.modules', ROOT=root), self.assertRaises(RuntimeError):
                    batch.verify(Path('fixture.root'), 'breakpoint')

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
