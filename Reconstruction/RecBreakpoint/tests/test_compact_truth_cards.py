"""Optional truth collections survive frozen cards, including legacy inputs."""
import importlib.util
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    'batch', Path(__file__).resolve().parents[1]/'options/batch_breakpoint.py')
batch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(batch)


class CompactTruthCardsTest(unittest.TestCase):
    def test_maintained_tracker_and_gsf_cards(self):
        repo=Path(__file__).resolve().parents[3]
        for name in ('DumpGsfTrks/trk.py.bk','DumpGsfTrks/gsf.py.bk'):
            text=(repo/name).read_text().split('# Companion truth is absent in legacy full-step files.\n')[1]
            text=text.split('_truth_input.Close()')[0]+'_truth_input.Close()'
            expected=list(batch.OPTIONAL_COMPACT_TRUTH) if name.endswith('trk.py.bk') else ['GsfG4MaterialStepSpans']
            for present in ((),batch.OPTIONAL_COMPACT_TRUTH):
                branches=[types.SimpleNamespace(GetName=lambda name=n: name) for n in present]
                tree=types.SimpleNamespace(GetListOfBranches=lambda: branches)
                file=types.SimpleNamespace(IsZombie=lambda:False,Get=lambda _:tree,Close=lambda:None)
                root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda *_:file))
                reader=types.SimpleNamespace(collections=[])
                context={'podioinput':reader,'input_file':'input.root','inputfilename':'input.root',
                         'dsvc':types.SimpleNamespace(input='input.root')}
                with patch.dict(sys.modules,ROOT=root):exec(text,context)
                self.assertEqual(reader.collections,expected if present else [])

    def test_optional_collections_for_each_reader(self):
        for reader in ('podioinput', 'inp'):
            for present in ((), batch.OPTIONAL_COMPACT_TRUTH):
                with self.subTest(reader=reader, present=present):
                    anchor = ('# ApplicationMgr\n' if reader == 'podioinput'
                              else 'from Configurables import ApplicationMgr\n')
                    text = batch.pass_through(anchor, reader, ())
                    # Execute only the inserted block, independently of Gaudi.
                    text = text.removesuffix(anchor)
                    branches = [types.SimpleNamespace(GetName=lambda name=n: name) for n in present]
                    tree = types.SimpleNamespace(GetListOfBranches=lambda: branches)
                    file = types.SimpleNamespace(IsZombie=lambda: False,
                                                 Get=lambda _: tree, Close=lambda: None)
                    root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda _: file))
                    value = types.SimpleNamespace(collections=['CompleteTracks'])
                    context = {reader: value, 'k4DataSvc': lambda _: types.SimpleNamespace(input='input.root')}
                    with patch.dict(sys.modules, ROOT=root):
                        exec(text, context)
                    self.assertEqual(value.collections, ['CompleteTracks']+list(present))


if __name__ == '__main__':
    unittest.main()
