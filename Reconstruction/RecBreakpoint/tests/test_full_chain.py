"""Check full-chain freezing and worker behavior without Gaudi or batch submission."""
import ast
import contextlib
import hashlib
import importlib.util
import io
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


class FullChainTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='breakpoint_full_chain_')
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name)
        for relative in ('DumpGsfTrks/sim.py.bk', 'DumpGsfTrks/trk.py.bk',
                         'DumpGsfTrks/calodigi.py.bk', 'DumpGsfTrks/rec.py.bk',
                         'DumpGsfTrks/full_chain.py.bk', 'dump_breakpoint.sh',
                         'Reconstruction/RecBreakpoint/options/run_breakpoint.py'):
            target = self.repo/relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(REPO/relative, target)

    def prepare(self, stages='sim,trk,calodigi,rec,breakpoint'):
        env = dict(CEPCSW_BREAKPOINT_DIR=str(self.repo), INPUT_TUPLEPATH='input',
                   OUTPUT_TUPLEPATH='output', STAGES=stages, NEVT='2', SEED_FIRST='1',
                   SEED_LAST='1', MEMORY_MB='5000', DRY_RUN='1', SAMPLE_REGION='barrel',
                   PARTICLES='e-', THETAS='85', TRANSVERSE_MOMENTA='2.0')
        with patch.dict(os.environ, env, clear=True), contextlib.redirect_stdout(io.StringIO()):
            batch.prepare()
        manifest = self.repo/'output/runcards/barrel-1/job.json'
        return manifest, json.loads(manifest.read_text())

    def test_frozen_chain_embeds_all_cards_in_physical_order(self):
        _, job = self.prepare()
        text = Path(job['cards']['full_chain']).read_text()
        cards = next(ast.literal_eval(node.value) for node in ast.parse(text).body
                     if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name)
                     and t.id == 'stage_cards' for t in node.targets))
        self.assertEqual(list(cards), list(batch.ORDER))
        for stage, source in cards.items():
            self.assertEqual(source, Path(job['cards'][stage]).read_text())
        self.assertEqual(hashlib.sha256(text.encode()).hexdigest(), job['checksums']['full_chain'])
        self.assertIn('_in_memory_chain', cards['trk'])
        self.assertIn('_in_memory_chain', cards['breakpoint'])

    def test_worker_runs_once_and_never_creates_or_deletes_intermediates(self):
        manifest, job = self.prepare()
        def execute(command, **kwargs):
            self.assertEqual(command[-1], job['cards']['full_chain'])
            for stage in ('rec', 'breakpoint'):
                Path(job['files'][stage]).write_bytes(b'fixture')
        with patch.object(Path, 'cwd', return_value=self.repo), \
             patch.object(batch.shutil, 'which', return_value='/runtime/gaudirun.py'), \
             patch.object(batch.subprocess, 'run', side_effect=execute) as run, \
             patch.object(batch, 'verify', return_value=True) as verify, \
             patch.object(batch, 'cleanup_intermediate') as cleanup:
            batch.run(manifest)
        run.assert_called_once()
        self.assertEqual([call.args[1] for call in verify.call_args_list], ['rec', 'breakpoint'])
        cleanup.assert_not_called()
        self.assertTrue(all(not Path(job['files'][stage]).exists() for stage in batch.ORDER[:3]))
        completed = json.loads((manifest.parent/'completed.json').read_text())
        self.assertEqual(set(completed['outputs']), {'rec', 'breakpoint'})
        self.assertTrue(all(value['status'] == 'not_written_in_memory'
                            for value in completed['intermediate_cleanup'].values()))

    def test_tampered_chain_is_rejected_before_start(self):
        manifest, job = self.prepare()
        Path(job['cards']['full_chain']).write_text('# changed\n')
        with patch.object(Path, 'cwd', return_value=self.repo), \
             patch.object(batch.shutil, 'which', return_value='/runtime/gaudirun.py'), \
             patch.object(batch.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'Frozen card changed'):
                batch.run(manifest)
        run.assert_not_called()
        self.assertFalse((manifest.parent/'started.json').exists())

    def test_failed_final_verification_does_not_mark_complete(self):
        manifest, _ = self.prepare()
        with patch.object(Path, 'cwd', return_value=self.repo), \
             patch.object(batch.shutil, 'which', return_value='/runtime/gaudirun.py'), \
             patch.object(batch.subprocess, 'run'), patch.object(batch, 'verify', return_value=False):
            with self.assertRaisesRegex(RuntimeError, 'verification failed: rec'):
                batch.run(manifest)
        self.assertFalse((manifest.parent/'completed.json').exists())

    def test_detector_chain_without_breakpoint_is_supported(self):
        _, job = self.prepare('sim,trk,calodigi,rec')
        self.assertIn('full_chain', job['cards'])
        self.assertNotIn('breakpoint', job['cards'])


if __name__ == '__main__':
    unittest.main()
