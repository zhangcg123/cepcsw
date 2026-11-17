"""Configuration isolation and complete output-key registration without Gaudi."""
import ast,copy,re,tempfile,unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[3]

class Handle:
    def __init__(self,path):self.path=path
    def mode(self):return 'W'
    def __str__(self):return self.path

class Fit:
    IncludeHcalEnergy=True
    def __init__(self,path):
        self.OutputFile=str(path);self.OutputTracks=Handle('Tracks');self.OutputStatus=Handle('Status')
        self.Selection=[3,11];self.OutputLevel=3
    def getName(self):return 'BP'
    def clone(self,name):return copy.deepcopy(self)
    def getDefaultProperties(self):return dict(OutputTracks=Handle('Tracks'),OutputStatus=Handle('Status'),OutputFile='x.root',OutputLevel=3)

class PairTest(unittest.TestCase):
    def function(self):
        path=ROOT/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        node=next(n for n in ast.parse(path.read_text()).body if isinstance(n,ast.FunctionDef) and n.name=='configure_calo_pair')
        namespace={};exec(compile(ast.Module(body=[node],type_ignores=[]),str(path),'exec'),namespace)
        return namespace['configure_calo_pair']
    def test_clone_preserves_primary_and_isolates_writers(self):
        with tempfile.TemporaryDirectory() as d:
            primary=Fit(Path(d)/'flat.root');control,main=self.function()(primary)
            self.assertIs(main,primary);self.assertTrue(main.IncludeHcalEnergy);self.assertFalse(control.IncludeHcalEnergy)
            self.assertEqual(str(main.OutputTracks),'Tracks');self.assertEqual(control.OutputTracks,'Tracks_EcalOnly')
            self.assertEqual(str(main.OutputStatus),'Status');self.assertEqual(control.OutputStatus,'Status_EcalOnly')
            self.assertEqual(control.OutputFile,str(Path(d)/'flat_ecal_only.root'))
            self.assertEqual(control.Selection,main.Selection);self.assertIsNot(control.Selection,main.Selection)
            self.assertEqual(control.OutputLevel,3)
            Path(control.OutputFile).touch()
            with self.assertRaisesRegex(ValueError,'overwrite'):self.function()(primary)
    def test_disabled_primary_and_old_install_fail_explicitly(self):
        primary=Fit('not-written.root');primary.IncludeHcalEnergy=False
        with self.assertRaisesRegex(ValueError,'BP_INCLUDE_HCAL'):self.function()(primary)
        primary.IncludeHcalEnergy=True;primary.getDefaultProperties=lambda:{}
        with self.assertRaisesRegex(ValueError,'installation'):self.function()(primary)
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
