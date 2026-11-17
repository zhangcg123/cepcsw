"""Single-instance calorimeter comparison steering and writer registration."""
import ast,re,unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[3]

class PairTest(unittest.TestCase):
    def test_card_uses_one_algorithm_and_explicit_comparison_switch(self):
        import os,types
        from unittest.mock import patch
        path=ROOT/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        tree=ast.parse(path.read_text())
        nodes=[n for n in tree.body if isinstance(n,ast.Assign) and any(
            isinstance(t,ast.Attribute) and isinstance(t.value,ast.Name) and
            t.value.id=='fit' and t.attr in ('RunBothCalo','IncludeHcalEnergy') for t in n.targets)]
        self.assertEqual(len(nodes),2)
        for flag in ('0','1'):
            fit=types.SimpleNamespace()
            with patch.dict(os.environ,{'BP_RUN_BOTH_CALO':flag},clear=True):
                exec(compile(ast.Module(body=nodes,type_ignores=[]),str(path),'exec'),{'fit':fit,'os':os})
            self.assertEqual(fit.RunBothCalo,flag=='1');self.assertTrue(fit.IncludeHcalEnergy)
        apps=[n for n in ast.walk(tree) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id=='ApplicationMgr']
        self.assertEqual(len(apps),1)
        top=next(k.value for k in apps[0].keywords if k.arg=='TopAlg')
        self.assertEqual(ast.unparse(top),'[reader, fit]')
        self.assertNotIn('.clone(',path.read_text())

    def test_all_24_writers_have_configurable_names(self):
        base=ROOT/'Reconstruction/RecBreakpoint/src'
        header=(base/'RecBreakpoint.h').read_text();source=(base/'RecBreakpoint.cpp').read_text()
        writers=set(re.findall(r'DataHandle<[^;]+?>\s+(m_\w+)\{[^;]+?Gaudi::DataHandle::Writer',header))
        registered=set(re.findall(r'declareProperty\("Output\w+",\s*(m_\w+)',source))
        self.assertIn('m_constraintOutputs',writers)
        writers.remove('m_constraintOutputs')
        self.assertEqual(len(writers)+6,24)
        self.assertTrue(writers<=registered,(writers-registered))
        self.assertIn('m_constraintOutputs[i]',source)

if __name__=='__main__':unittest.main()
