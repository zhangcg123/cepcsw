"""REC omits raw calorimeter simulation while retaining breakpoint inputs."""
import ast
import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch

REPO=Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('batch',REPO/'Reconstruction/RecBreakpoint/options/batch_breakpoint.py')
batch=importlib.util.module_from_spec(spec)
spec.loader.exec_module(batch)


class RecOutputPolicyTest(unittest.TestCase):
    def test_policy_only_omits_raw_simulation_and_its_links(self):
        original=(REPO/'DumpGsfTrks/rec.py.bk').read_text()
        result=batch.rec_output_policy(original)
        assignments=[node for node in ast.walk(ast.parse(result)) if isinstance(node,ast.Assign)
                     and any(isinstance(target,ast.Attribute) and target.attr=='outputCommands'
                             for target in node.targets)]
        commands=ast.literal_eval(assignments[0].value)
        self.assertEqual(commands,['keep *']+['drop '+name for name in batch.REC_DROP_COLLECTIONS])
        self.assertEqual(len(batch.REC_DROP_COLLECTIONS),12)
        preserved={'ECALBarrel','ECALEndcaps','HCALBarrel','HCALEndcaps',
                   'ECALBarrelParticleAssoCol','ECALEndcapsParticleAssoCol',
                   'HCALBarrelParticleAssoCol','HCALEndcapsParticleAssoCol',
                   'CyberPFOPreReclustering','EcalClusterPreReclustering','EcalHitsPreReclustering',
                   'HcalClusterPreReclustering','HcalHitsPreReclustering',
                   'CyberPFOPID','EcalCluster','CompleteTracks','MCParticle','MCParticleGen'}
        self.assertFalse(set(batch.REC_DROP_COLLECTIONS)&(preserved|set(batch.BREAKPOINT_INPUT_COLLECTIONS)|set(batch.OPTIONAL_COMPACT_TRUTH)))
        self.assertIn('out.outputCommands = ["keep *"]',original)
        with self.assertRaisesRegex(ValueError,'Card template changed'):
            batch.rec_output_policy(result)

    def test_verification_checks_policy_without_rejecting_legacy_inputs(self):
        for present in ((),('EcalBarrelCollection',)):
            closed=[]
            tree=types.SimpleNamespace(GetEntries=lambda:2,GetBranch=lambda name:name in present)
            file=types.SimpleNamespace(IsZombie=lambda:False,TestBit=lambda _:False,
                                       Get=lambda _:tree,Close=lambda:closed.append(True))
            root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda _:file,kRecovered=1))
            with patch.dict(sys.modules,ROOT=root),contextlib.redirect_stdout(io.StringIO()):
                self.assertTrue(batch.verify(Path('input.root'),'sim'))
                if present:
                    with self.assertRaisesRegex(RuntimeError,'retained omitted calorimeter'):
                        batch.verify(Path('input.root'),'sim',forbidden_collections=batch.REC_DROP_COLLECTIONS)
                else:
                    self.assertTrue(batch.verify(Path('input.root'),'sim',forbidden_collections=batch.REC_DROP_COLLECTIONS))
            self.assertEqual(len(closed),2)


if __name__=='__main__':unittest.main()
