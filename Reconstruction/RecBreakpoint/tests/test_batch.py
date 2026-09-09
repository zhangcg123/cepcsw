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
                         dict(INPUT_TUPLEPATH='outputs'),dict(BP_TRUTH_OVERRIDE='maybe'),dict(SEED_LAST='13')]:
            with self.subTest(override=override),self.assertRaises(ValueError): self.prepare(**override)
            self.assertFalse((self.repo/'outputs').exists())

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


if __name__ == '__main__': unittest.main()
