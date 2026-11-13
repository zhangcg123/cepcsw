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
    def test_eehx_wrapper_full_chain_with_truth_electron_selection(self):
        launcher = self.repo/'subbreakpointhiggsjobs.sh'
        shutil.copy2(REPO/launcher.name, launcher)
        source = self.repo/'stdhep_eehx'; source.mkdir()
        for index in (1, 2):
            (source/f'eehx.{index:04d}.stdhep').write_bytes(b'STDHEP fixture')
        env = dict(self.env, PATH=os.environ.get('PATH', '/usr/bin:/bin'),
                   STDHEP_INPUT_PATTERN=str(source/'eehx.{seed:04d}.stdhep'),
                   NEVT='200', SEED_FIRST='1', SEED_LAST='2')
        result = batch.subprocess.run(['bash', str(launcher)], env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        for index in (1, 2):
            carddir = self.repo/f'outputs/runcards/higgs-{index}'
            job = json.loads((carddir/'job.json').read_text())
            self.assertEqual(job['stages'], ['sim', 'trk', 'calodigi', 'rec', 'breakpoint'])
            self.assertEqual(job['nevt'], 200)
            self.assertEqual(job['controls']['BP_SELECT_TRUTH_PRODUCTION_ELECTRONS'], '1')
            self.assertEqual(job['controls']['BP_LOCATION_SCAN_SETUPS'], '')
            self.assertEqual(job['controls']['BP_NEUTRAL_THETA_MRAD'], '25')
            self.assertEqual(job['controls']['BP_NEUTRAL_PHI_MRAD'], '200')
            self.assertIn('genalg.GenTools = ["StdHepRdr"]', (carddir/'sim.py').read_text())
            self.assertIn("'BP_SELECT_TRUTH_PRODUCTION_ELECTRONS': '1'", (carddir/'breakpoint.py').read_text())
            self.assertFalse((carddir/'submitted.json').exists())
        result = batch.subprocess.run(['bash', str(launcher), 'submit-prepared'],
                                      env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertIn('-g cms ', result.stdout)
        self.assertFalse(list((self.repo/'outputs/runcards').glob('*/submitted.json')))
        self.assertIn('SEED_LAST=${SEED_LAST:-500}', launcher.read_text())
        self.assertIn('E240.Pe1e1h_X.e0.p0.whizard195', launcher.read_text())

    def test_truth_production_electron_selection_is_optional_and_frozen(self):
        self.prepare(OUTPUT_TUPLEPATH='selection_default')
        self.assertEqual(self.manifest('selection_default')['controls']['BP_SELECT_TRUTH_PRODUCTION_ELECTRONS'], '1')
        self.prepare(OUTPUT_TUPLEPATH='selection_off', BP_SELECT_TRUTH_PRODUCTION_ELECTRONS='0')
        self.assertEqual(self.manifest('selection_off')['controls']['BP_SELECT_TRUTH_PRODUCTION_ELECTRONS'], '0')
        self.prepare(OUTPUT_TUPLEPATH='selection_on', BP_SELECT_TRUTH_PRODUCTION_ELECTRONS='true')
        job = self.manifest('selection_on')
        self.assertEqual(job['controls']['BP_SELECT_TRUTH_PRODUCTION_ELECTRONS'], '1')
        self.assertIn('fit.SelectTruthProductionElectrons =', Path(job['cards']['breakpoint']).read_text())
        with self.assertRaisesRegex(ValueError, 'Expected a boolean'):
            self.prepare(OUTPUT_TUPLEPATH='selection_invalid', BP_SELECT_TRUTH_PRODUCTION_ELECTRONS='maybe')

    def test_fit_defaults_and_disabled_scans_in_card_and_both_wrappers(self):
        import ast
        properties = {'FreeLossFit': True, 'BeamSpotConstraint': True,
                      'ExperimentalBeamSpotFsrLoss': True,
                      'ChargedEcalMeasurement': True, 'NeutralEcalMeasurement': False,
                      'LocationScanSetups': [], 'ChargedEcalEnergyMode': 'ElectronPhoton'}
        expected = {'BP_FREE_LOSS_FIT':'1', 'BP_BEAM_SPOT_CONSTRAINT':'1',
                    'BP_BEAM_SPOT_FSR_LOSS':'1',
                    'BP_CHARGED_ECAL_MEASUREMENT':'1', 'BP_NEUTRAL_ECAL_MEASUREMENT':'0',
                    'BP_LOCATION_SCAN_SETUPS':'', 'BP_CHARGED_ECAL_ENERGY_MODE':'ElectronPhoton'}
        text=(REPO/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        nodes=[]
        for node in ast.parse(text).body:
            if isinstance(node,ast.Assign) and any(
                    isinstance(t,ast.Attribute) and isinstance(t.value,ast.Name)
                    and t.value.id=='fit' and t.attr in properties or
                    isinstance(t,ast.Name) and t.id=='location_scan_setups' for t in node.targets):
                nodes.append(node)
            elif isinstance(node,ast.If) and ast.unparse(node.test)=="'LocationScanSetups' in fit.getDefaultProperties()":
                nodes.append(node)
        fit=types.SimpleNamespace(getDefaultProperties=lambda: {'LocationScanSetups':[110]})
        with patch.dict(os.environ,{},clear=True):
            exec(compile(ast.Module(body=nodes,type_ignores=[]),'<default-steering>','exec'), {'fit':fit,'os':os})
        for key,value in properties.items(): self.assertEqual(getattr(fit,key),value)
        header=(REPO/'Reconstruction/RecBreakpoint/src/RecBreakpoint.h').read_text()
        for key,value in properties.items():
            if isinstance(value,list): literal=r'\{110\}'  # Compiled default; the card overrides it to [].
            elif isinstance(value,str): literal='"'+value+'"'
            else: literal='true' if value else 'false'
            self.assertRegex(header,r'"'+key+r'",\s*'+literal+r'\}')
        self.prepare(OUTPUT_TUPLEPATH='direct_defaults')
        direct=self.manifest('direct_defaults')
        self.assertEqual({k:direct['controls'][k] for k in expected},expected)
        (self.repo/'inputs/rec-higgs-1.root').write_bytes(b'planning fixture')
        for script,sample in [('subbreakpointjobs.sh','barrel-12'),('subbreakpointhiggsscanjobs.sh','higgs-1')]:
            output='defaults_'+script
            env=dict(self.env,STAGES='breakpoint',SAMPLE_REGION='barrel',OUTPUT_TUPLEPATH=output)
            result=batch.subprocess.run(['bash',str(self.repo/script)],env=env,capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            job=json.loads((self.repo/output/'runcards'/sample/'job.json').read_text())
            self.assertEqual({k:job['controls'][k] for k in expected},expected)
            off={key:'0' for key in expected}
            off['BP_LOCATION_SCAN_SETUPS']=''
            off['BP_CHARGED_ECAL_ENERGY_MODE']='ElectronOnly'
            off_output=output+'_off'
            result=batch.subprocess.run(['bash',str(self.repo/script)],
                env=dict(env,OUTPUT_TUPLEPATH=off_output,**off),capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            disabled=json.loads((self.repo/off_output/'runcards'/sample/'job.json').read_text())
            self.assertEqual({k:disabled['controls'][k] for k in off},off)

    def test_neutral_window_defaults_and_override_in_card_and_wrappers(self):
        import ast
        (self.repo/'inputs/rec-higgs-1.root').write_bytes(b'planning fixture')
        text = (REPO/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        nodes = [node for node in ast.parse(text).body if isinstance(node, ast.Assign)
                 and any(isinstance(t, ast.Attribute) and isinstance(t.value, ast.Name)
                         and t.value.id == 'fit' and t.attr in
                         ('NeutralLossThetaWindowMrad', 'NeutralLossPhiWindowMrad')
                         for t in node.targets)]
        for override, theta in [({}, '25'), ({'BP_NEUTRAL_THETA_MRAD': '50'}, '50')]:
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, override, clear=True):
                exec(compile(ast.Module(body=nodes, type_ignores=[]), '<neutral-window>', 'exec'),
                     {'fit': fit, 'os': os})
            self.assertEqual(fit.NeutralLossThetaWindowMrad, float(theta))
            self.assertEqual(fit.NeutralLossPhiWindowMrad, 200.)
            output = 'window_direct_' + theta
            self.prepare(OUTPUT_TUPLEPATH=output, **override)
            self.assertEqual(self.manifest(output)['controls']['BP_NEUTRAL_THETA_MRAD'], theta)
            for script, sample in [('subbreakpointjobs.sh', 'barrel-12'),
                                   ('subbreakpointhiggsscanjobs.sh', 'higgs-1')]:
                output = script + '_window_' + theta
                env = dict(self.env, STAGES='breakpoint', SAMPLE_REGION='barrel',
                           OUTPUT_TUPLEPATH=output, **override)
                result = batch.subprocess.run(['bash', str(self.repo/script)], env=env,
                                              capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                job = json.loads((self.repo/output/'runcards'/sample/'job.json').read_text())
                self.assertEqual(job['controls']['BP_NEUTRAL_THETA_MRAD'], theta)
                self.assertEqual(job['controls']['BP_NEUTRAL_PHI_MRAD'], '200')

    def test_floating_fsr_switch_is_validated_and_frozen(self):
        self.prepare(OUTPUT_TUPLEPATH='fsr_on', BP_BEAM_SPOT_FSR_LOSS='true')
        self.assertEqual(self.manifest('fsr_on')['controls']['BP_BEAM_SPOT_FSR_LOSS'], '1')
        self.prepare(OUTPUT_TUPLEPATH='fsr_off', BP_BEAM_SPOT_FSR_LOSS='0')
        self.assertEqual(self.manifest('fsr_off')['controls']['BP_BEAM_SPOT_FSR_LOSS'], '0')
        with self.assertRaisesRegex(ValueError, 'Expected a boolean'):
            self.prepare(OUTPUT_TUPLEPATH='fsr_bad', BP_BEAM_SPOT_FSR_LOSS='maybe')
        with self.assertRaisesRegex(ValueError, 'HitScanAll requires BP_BEAM_SPOT_FSR_LOSS=0'):
            self.prepare(OUTPUT_TUPLEPATH='fsr_scan', BP_INTERVAL_SELECTION_MODE='HitScanAll')

    def test_truth_endpoints_and_scan_setups_are_independently_frozen(self):
        self.prepare(BP_INTERVAL_SELECTION_MODE='Truth', BP_FREE_LOSS_FIT='1',
                     BP_BEAM_SPOT_CONSTRAINT='1', BP_CHARGED_ECAL_MEASUREMENT='1',
                     BP_LOCATION_SCAN_SETUPS='110,0')
        job = self.manifest()
        self.assertEqual(job['controls']['BP_LOCATION_SCAN_SETUPS'], '0,110')
        self.assertEqual(job['controls']['BP_INTERVAL_SELECTION_MODE'], 'Truth')
        self.assertEqual(job['controls']['BP_BEAM_SPOT_CONSTRAINT'], '1')
        self.assertIn('fit.LocationScanSetups = location_scan_setups',
                      Path(job['cards']['breakpoint']).read_text())
        for invalid in ('110,110', '10'):
            with self.assertRaisesRegex(ValueError, 'unique'):
                self.prepare(OUTPUT_TUPLEPATH='bad_'+invalid.replace(',','_'), BP_LOCATION_SCAN_SETUPS=invalid)
        with self.assertRaisesRegex(ValueError, 'requires Truth'):
            self.prepare(OUTPUT_TUPLEPATH='bad_mode', BP_INTERVAL_SELECTION_MODE='HitScanAll', BP_LOCATION_SCAN_SETUPS='0')
        with self.assertRaisesRegex(ValueError, 'BP_BEAM_SPOT_CONSTRAINT'):
            self.prepare(OUTPUT_TUPLEPATH='missing_beam', BP_LOCATION_SCAN_SETUPS='100', BP_BEAM_SPOT_CONSTRAINT='0')
        with self.assertRaisesRegex(ValueError, 'BP_NEUTRAL_ECAL_MEASUREMENT'):
            self.prepare(OUTPUT_TUPLEPATH='missing_neutral', BP_LOCATION_SCAN_SETUPS='111',
                         BP_BEAM_SPOT_CONSTRAINT='1', BP_CHARGED_ECAL_MEASUREMENT='1', BP_NEUTRAL_ECAL_MEASUREMENT='0')

    def test_truth_scan_verification_requires_scan_schema_without_scan_only_mode(self):
        tree=MagicMock(); tree.GetEntries.return_value=2
        tree.GetBranch.side_effect=lambda name: None if name=='location_scan_setup' else object()
        file=MagicMock(); file.Get.return_value=tree
        file.IsZombie.return_value=False; file.TestBit.return_value=False
        root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path:file,kRecovered=1))
        with patch.dict('sys.modules',ROOT=root):
            with self.assertRaisesRegex(RuntimeError,'Missing location-scan branch'):
                batch.verify(Path('fixture.root'),'breakpoint',hit_scan='TruthWithLocationScan')

    def test_resolved_ecal_mode_removed(self):
        self.prepare()
        job=self.manifest()
        self.assertNotIn('BP_RESOLVED_ECAL_ENERGY_MODE',job['controls'])
        self.assertNotIn('fit.ResolvedEcalEnergyMode',Path(job['cards']['breakpoint']).read_text())
        for stale in ('Separate','Sum'):
            with self.assertRaisesRegex(ValueError,'was removed'):
                self.prepare(OUTPUT_TUPLEPATH='stale_'+stale,BP_RESOLVED_ECAL_ENERGY_MODE=stale)

    def test_resolved_ecal_output_requires_applied_flag(self):
        for missing in ('charged_ecal_free_loss_rts_resolved_ecal_sum_applied','neutral_ecal_free_loss_rts_resolved_ecal_sum_applied'):
            with self.subTest(missing=missing):
                tree=MagicMock(); tree.GetEntries.return_value=2
                tree.GetBranch.side_effect=lambda name: None if name==missing else object()
                file=MagicMock(); file.Get.return_value=tree
                file.IsZombie.return_value=False; file.TestBit.return_value=False
                root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path:file,kRecovered=1))
                with patch.dict('sys.modules',ROOT=root):
                    with self.assertRaisesRegex(RuntimeError,missing):
                        batch.verify(Path('fixture.root'),'breakpoint')

    def test_retired_hit_scan_requests_are_rejected(self):
        import runpy
        for controls in ({'BP_INTERVAL_SELECTION_MODE': 'HitScan'},
                         {'BP_HIT_SCAN_MIN_NLL_GAIN': '0'}):
            with self.subTest(controls=controls):
                with self.assertRaisesRegex(ValueError, 'were removed; use HitScanAll'):
                    self.prepare(STAGES='breakpoint', **controls)
                with patch.dict(os.environ, controls, clear=True):
                    with self.assertRaisesRegex(ValueError, 'were removed; use HitScanAll'):
                        runpy.run_path(str(self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'))
        self.assertFalse((self.repo/'outputs/runcards').exists())

    def test_scan_only_controls_are_frozen_without_retired_threshold(self):
        self.prepare(STAGES='breakpoint', BP_INTERVAL_SELECTION_MODE='HitScanAll',
                     BP_TRUTH_OVERRIDE='0', BP_LOCATION_SCAN_SETUPS='', BP_BEAM_SPOT_FSR_LOSS='0')
        job = self.manifest()
        self.assertEqual(job['controls']['BP_INTERVAL_SELECTION_MODE'], 'HitScanAll')
        self.assertNotIn('BP_HIT_SCAN_MIN_NLL_GAIN', job['controls'])
        card = Path(job['cards']['breakpoint']).read_text()
        self.assertNotIn('fit.HitScanMinNllGain', card)

    def test_scan_only_verification_requires_all_setup_schema(self):
        tree=MagicMock(); tree.GetEntries.return_value=2
        tree.GetBranch.side_effect=lambda name: None if name=='location_scan_setup' else object()
        file=MagicMock(); file.Get.return_value=tree
        file.IsZombie.return_value=False; file.TestBit.return_value=False
        root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path:file,kRecovered=1))
        with patch.dict('sys.modules',ROOT=root):
            with self.assertRaisesRegex(RuntimeError,'Missing scan-only branch: location_scan_setup'):
                batch.verify(Path('fixture.root'),'breakpoint',hit_scan='HitScanAll')

    def test_scan_only_verification_requires_full_ip_state(self):
        for missing in ('location_scan_ip_parameters','location_scan_ip_covariance',
                        'location_scan_ip_reference_point','location_scan_ip_time','location_scan_ip_location'):
            with self.subTest(missing=missing):
                tree=MagicMock(); tree.GetEntries.return_value=2
                tree.GetBranch.side_effect=lambda name: None if name==missing else object()
                file=MagicMock(); file.Get.return_value=tree
                file.IsZombie.return_value=False; file.TestBit.return_value=False
                root=types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path:file,kRecovered=1))
                with patch.dict('sys.modules',ROOT=root):
                    with self.assertRaisesRegex(RuntimeError,'Missing scan-only branch: '+missing):
                        batch.verify(Path('fixture.root'),'breakpoint',hit_scan='HitScanAll')

    def test_scan_only_reads_truth_associations_without_loss_override(self):
        import ast
        text=(self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        block=next(n for n in ast.parse(text).body if isinstance(n,ast.If)
                   and ast.unparse(n.test).startswith("fit.IntervalSelectionMode in ('Truth', 'HitScanAll')"))
        reader=types.SimpleNamespace(collections=[])
        fit=types.SimpleNamespace(IntervalSelectionMode='HitScanAll',TruthOverride=False)
        exec(compile(ast.Module(body=[block],type_ignores=[]),'<truth-inputs>','exec'),
             {'fit':fit,'reader':reader})
        self.assertIn('VXDTrackerHitAssociation',reader.collections)
        self.assertIn('TPCTrackerHitAss',reader.collections)

    def test_higgs_scan_launcher_prepares_all_rec_events_and_submits_only_on_request(self):
        import subprocess
        for seed in range(1,251):
            if seed!=123:
                (self.repo/'inputs'/f'rec-higgs-{seed}.root').write_bytes(b'planning fixture')
        launcher=str(self.repo/'subbreakpointhiggsscanjobs.sh')
        result=subprocess.run([launcher],env=self.env,capture_output=True,text=True,check=True)
        self.assertIn('Prepared 249 jobs: breakpoint',result.stdout)
        self.assertIn('Skipped 1 samples',result.stdout)
        self.assertIn('-g cms ',result.stdout)
        manifests=list((self.repo/'outputs/runcards').glob('*/job.json'))
        self.assertEqual(len(manifests),249)
        jobs=[json.loads(p.read_text()) for p in manifests]
        self.assertEqual(sum(j['nevt'] for j in jobs),49800)
        for job in jobs:
            self.assertEqual(job['nevt'],200)
            self.assertEqual(job['stages'],['breakpoint'])
            self.assertEqual(job['controls']['BP_INTERVAL_SELECTION_MODE'],'Truth')
            self.assertEqual(job['controls']['BP_LOCATION_SCAN_SETUPS'],'')
            self.assertEqual(job['controls']['BP_TRUTH_OVERRIDE'],'0')
            self.assertEqual(job['controls']['BP_NEUTRAL_ECAL_MEASUREMENT'],'0')
            self.assertEqual(job['controls']['BP_EXPERIMENTAL_DELAYED_NEUTRAL'],'1')
        self.assertNotIn(123,[j['seed'] for j in jobs])
        self.assertIn(250,[j['seed'] for j in jobs])
        self.assertFalse(list((self.repo/'outputs').rglob('submitted.json')))
        subprocess.run([launcher,'submit-prepared'],env=self.env,capture_output=True,text=True,check=True)
        self.assertFalse(list((self.repo/'outputs').rglob('submitted.json')))

    def test_delayed_neutral_is_validated_and_frozen(self):
        template = (self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        assignment = next(line for line in template.splitlines()
                          if line.startswith('fit.ExperimentalDelayedNeutral ='))
        for value, expected in [('true', True), ('false', False)]:
            output = 'delayed_' + value
            self.prepare(STAGES='breakpoint', OUTPUT_TUPLEPATH=output,
                         BP_EXPERIMENTAL_DELAYED_NEUTRAL=value)
            job = self.manifest(output)
            self.assertEqual(job['controls']['BP_EXPERIMENTAL_DELAYED_NEUTRAL'], str(int(expected)))
            card = Path(job['cards']['breakpoint']).read_text()
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, {'BP_EXPERIMENTAL_DELAYED_NEUTRAL': str(int(not expected))}, clear=True):
                exec(card[:-len(template)] + '\n' + assignment, {'fit': fit, 'os': os})
            self.assertEqual(fit.ExperimentalDelayedNeutral, expected)
        with self.assertRaisesRegex(ValueError, 'Expected a boolean'):
            self.prepare(OUTPUT_TUPLEPATH='invalid', BP_EXPERIMENTAL_DELAYED_NEUTRAL='typo')
        self.assertFalse((self.repo/'invalid').exists())

    def test_sum_mode_allows_neutral_for_trackwise_routing(self):
        self.prepare(BP_CHARGED_ECAL_MEASUREMENT='1', BP_NEUTRAL_ECAL_MEASUREMENT='1',
                     BP_CHARGED_ECAL_ENERGY_MODE='ElectronPhoton')
        controls = self.manifest()['controls']
        self.assertEqual(controls['BP_CHARGED_ECAL_ENERGY_MODE'], 'ElectronPhoton')
        self.assertEqual(controls['BP_NEUTRAL_ECAL_MEASUREMENT'], '1')

    def test_ecal_input_default_and_legacy_are_frozen(self):
        template = (self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        assignments = '\n'.join(line for line in template.splitlines() if line.startswith(
            ('fit.EcalConstraintInput =', 'fit.ChargedEcalInputClusters =')))
        self.prepare()
        job = self.manifest()
        self.assertEqual(job['controls']['BP_ECAL_CONSTRAINT_INPUT'], 'PreReclustering')
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, job['controls'], clear=True):
            exec(assignments, {'fit': fit, 'os': os})
        self.assertEqual(fit.EcalConstraintInput, 'PreReclustering')
        self.assertEqual(fit.ChargedEcalInputClusters, '')
        self.prepare(OUTPUT_TUPLEPATH='legacy', BP_ECAL_CONSTRAINT_INPUT='FinalPFO',
                     BP_CHARGED_ECAL_INPUT_CLUSTERS='TrkMergedECAL')
        legacy = self.manifest('legacy')
        self.assertEqual(legacy['controls']['BP_ECAL_CONSTRAINT_INPUT'], 'FinalPFO')
        self.assertEqual(legacy['controls']['BP_CHARGED_ECAL_INPUT_CLUSTERS'], 'TrkMergedECAL')
        with self.assertRaisesRegex(ValueError, 'BP_ECAL_CONSTRAINT_INPUT'):
            self.prepare(OUTPUT_TUPLEPATH='invalid', BP_ECAL_CONSTRAINT_INPUT='typo')

    def test_compiled_and_frozen_card_tracking_defaults_agree(self):
        import re
        self.prepare()
        card = Path(self.manifest()['cards']['breakpoint']).read_text()
        template = (self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        assignments = '\n'.join(line for line in template.splitlines() if line.startswith(
            ('fit.LossStateMode =', 'fit.DiffuseAugmentedRTS =')))
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, {'BP_LOSS_STATE_MODE': 'Persistent6D',
                                     'BP_DIFFUSE_AUGMENTED_RTS': '0'}, clear=True):
            exec(card[:-len(template)] + '\n' + assignments, {'fit': fit, 'os': os})
        expected = {'LossStateMode': 'Persistent6D',
                    'DiffuseAugmentedRTS': True}
        header = (REPO/'Reconstruction/RecBreakpoint/src/RecBreakpoint.h').read_text()
        for name, value in expected.items():
            self.assertEqual(getattr(fit, name), value)
            literal = 'true' if value is True else '"' + value + '"'
            self.assertRegex(header, r'"' + name + r'",\s*' + re.escape(literal) + r'\}')

    def test_retired_ecal_controls_are_rejected_before_outputs(self):
        import runpy
        card = self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        for name in ('BP_ABSOLUTE_NEUTRAL_RTS', 'BP_ABSOLUTE_NEUTRAL_DIFFUSE_REFERENCE',
                     'BP_ABSOLUTE_NEUTRAL_REFERENCE_SOURCE', 'BP_ECAL_LOSS_REFERENCE_MODE',
                     'BP_FREE_LOSS_ECAL'):
            self.assertNotIn(name, batch.BP_CONTROLS)
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'neutral-cluster-constrained'):
                self.prepare(**{name: '1'})
            self.assertFalse((self.repo/'outputs').exists())
            with patch.dict(os.environ, {name: '1'}, clear=True):
                with self.assertRaisesRegex(ValueError, 'neutral-cluster-constrained'):
                    runpy.run_path(str(card))

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

    def test_gaussian_prior_and_persistent_state_remain(self):
        self.prepare()
        card = Path(self.manifest()['cards']['breakpoint']).read_text()
        assignments = '\n'.join(line for line in card.splitlines()
                                if line.startswith(('fit.MeanLogLoss =', 'fit.SigmaLogLoss =', 'fit.LossStateMode =')))
        for mode in (None, 'Persistent6D'):
            fit = types.SimpleNamespace()
            overrides = {} if mode is None else {'BP_LOSS_STATE_MODE': mode}
            with self.subTest(mode=mode), patch.dict(os.environ, overrides, clear=True):
                exec(assignments, {'fit':fit, 'os':os})
            self.assertEqual(fit.LossStateMode, mode or 'Persistent6D')
            self.assertEqual(fit.MeanLogLoss, 0)
            self.assertEqual(fit.SigmaLogLoss, .001)
        self.assertNotIn('fit.LossPriorMode', card)

    def test_local_marginal_is_rejected_before_outputs(self):
        import runpy
        with self.assertRaisesRegex(ValueError, 'LocalMarginal was removed'):
            self.prepare(BP_LOSS_STATE_MODE='LocalMarginal')
        self.assertFalse((self.repo/'outputs').exists())
        card = self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        with patch.dict(os.environ, {'BP_LOSS_STATE_MODE': 'LocalMarginal'}, clear=True):
            with self.assertRaisesRegex(ValueError, 'LocalMarginal was removed'):
                runpy.run_path(str(card))

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
                    'DumpGsfTrks/calodigi.py.bk', 'DumpGsfTrks/rec.py.bk',
                    'DumpGsfTrks/full_chain.py.bk',
                    'Reconstruction/RecBreakpoint/options/run_breakpoint.py', 'dump_breakpoint.sh',
                    'subbreakpointjobs.sh', 'subbreakpointhiggsscanjobs.sh',
                    'Reconstruction/RecBreakpoint/options/batch_breakpoint.py']:
            dest = self.repo/rel; dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(REPO/rel, dest)
        (self.repo/'inputs').mkdir()
        for stage in ['sim','trk','calodigi','rec']:
            (self.repo/'inputs'/f'{stage}-e--2.0-85-12.root').write_bytes(b'planning fixture')
            (self.repo/'inputs'/f'{stage}-barrel-12.root').write_bytes(b'planning fixture')
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
                for stage in ('sim','trk','calodigi','rec'):
                    self.assertEqual(Path(job['files'][stage]).parent,self.repo/(out if stage in selected else 'inputs'))
                if 'breakpoint' in selected:
                    text=Path(job['cards']['breakpoint']).read_text()
                    self.assertIn(repr(job['files']['rec']),text)
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

    def test_backward_fit_switches_default_off_and_freeze_independently(self):
        names = {
            'OrdinaryBackwardFilter': 'BP_ORDINARY_BACKWARD_FILTER',
            'FreeLossBackwardFilter': 'BP_FREE_LOSS_BACKWARD_FILTER',
            'TruthOverrideBackwardFilter': 'BP_TRUTH_BACKWARD_FILTER',
        }
        self.prepare(OUTPUT_TUPLEPATH='backward_default')
        template = (self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
        assignments = '\n'.join(line for line in template.splitlines()
                                if any(line.startswith('fit.' + name + ' =') for name in names))
        for name in names:
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, {}, clear=True):
                exec(assignments, {'fit': fit, 'os': os})
            self.assertFalse(getattr(fit, name))
            output = 'backward_' + name
            self.prepare(OUTPUT_TUPLEPATH=output, **{names[name]: 'true'})
            job = self.manifest(output)
            self.assertEqual(job['controls'][names[name]], '1')
            frozen = Path(job['cards']['breakpoint']).read_text()
            prefix = frozen[:-len(template)]
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, {names[name]: '0'}, clear=True):
                exec(prefix + '\n' + assignments, {'fit': fit, 'os': os})
            self.assertTrue(getattr(fit, name))

    def test_controls_frozen_and_originals_unchanged(self):
        source = self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py'
        before = source.read_bytes()
        self.prepare(BP_TRUTH_OVERRIDE='true', BP_INTERVAL_SELECTION_MODE='Manual', BP_INTERVALS='5', BP_BACKWARD_SEED_SCALE='100', BP_LOCATION_SCAN_SETUPS='')
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
        self.assertNotIn('OutputTracksFreeLossEcal', card)
        self.assertNotIn('OutputTracksAbsoluteNeutralRTS', card)
        self.assertNotIn('OutputTracksNeutralFreeLossRTS', card)
        assignments = '\n'.join(line for line in card.splitlines()
                                if line.startswith(('fit.FreeLoss', 'fit.ChargedEcalMeasurement',
                                                    'fit.NeutralEcalMeasurement')))
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, {}, clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertTrue(fit.FreeLossFit)
        self.assertTrue(fit.ChargedEcalMeasurement)
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
        self.prepare(OUTPUT_TUPLEPATH='free_disabled', BP_FREE_LOSS_FIT='false', BP_LOCATION_SCAN_SETUPS='')
        disabled = self.manifest('free_disabled')
        self.assertEqual(disabled['controls']['BP_FREE_LOSS_FIT'], '0')
        with patch.dict(os.environ, disabled['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertFalse(fit.FreeLossFit)
        self.prepare(OUTPUT_TUPLEPATH='charged_ecal_enabled',
                     BP_CHARGED_ECAL_MEASUREMENT='true')
        charged = self.manifest('charged_ecal_enabled')
        self.assertEqual(charged['controls']['BP_CHARGED_ECAL_MEASUREMENT'], '1')
        with patch.dict(os.environ, charged['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertTrue(fit.ChargedEcalMeasurement)
        self.prepare(OUTPUT_TUPLEPATH='neutral_ecal_enabled',
                     BP_NEUTRAL_ECAL_MEASUREMENT='true',
                     BP_CHARGED_ECAL_MEASUREMENT='true')
        neutral = self.manifest('neutral_ecal_enabled')
        self.assertEqual(neutral['controls']['BP_NEUTRAL_ECAL_MEASUREMENT'], '1')
        with patch.dict(os.environ, neutral['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertTrue(fit.NeutralEcalMeasurement)
        self.assertTrue(fit.ChargedEcalMeasurement)
        self.prepare(OUTPUT_TUPLEPATH='ordinary_ecal_enabled', BP_FREE_LOSS_FIT='false',
                     BP_CHARGED_ECAL_MEASUREMENT='true', BP_NEUTRAL_ECAL_MEASUREMENT='true', BP_LOCATION_SCAN_SETUPS='')
        ordinary = self.manifest('ordinary_ecal_enabled')
        with patch.dict(os.environ, ordinary['controls'], clear=True):
            exec(assignments, {'fit':fit, 'os':os})
        self.assertFalse(fit.FreeLossFit)
        self.assertTrue(fit.ChargedEcalMeasurement)
        self.assertTrue(fit.NeutralEcalMeasurement)
        for retired in ('BP_FREE_LOSS_CHARGED_ECAL_MEASUREMENT', 'BP_FREE_LOSS_NEUTRAL_ECAL_MEASUREMENT'):
            with self.assertRaisesRegex(ValueError, 'was renamed'):
                self.prepare(OUTPUT_TUPLEPATH='retired_ecal', **{retired: '1'})
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

    def test_shared_beam_measurement_default_on_and_freeze(self):
        self.prepare(OUTPUT_TUPLEPATH='beam_default')
        job = self.manifest('beam_default')
        card = Path(job['cards']['breakpoint']).read_text()
        selected = ('fit.BeamSpotConstraint =', 'fit.BeamSpotX =', 'fit.BeamSpotY =',
                    'fit.BeamSpotSigmaX =', 'fit.BeamSpotSigmaY =')
        assignments = '\n'.join(line for line in card.splitlines() if line.startswith(selected))
        fit = types.SimpleNamespace()
        with patch.dict(os.environ, job['controls'], clear=True):
            exec(assignments, {'fit': fit, 'os': os})
        self.assertTrue(fit.BeamSpotConstraint)
        self.assertEqual((fit.BeamSpotX, fit.BeamSpotY), (0, 0))
        self.assertEqual((fit.BeamSpotSigmaX, fit.BeamSpotSigmaY), (.0145, 3.6e-5))
        self.assertNotIn('OutputTracksBeamGuidedFreeLossRTS', card)
        self.assertNotIn('OutputTracksBeamGuidedFreeLossBackwardFilter', card)
        self.prepare(OUTPUT_TUPLEPATH='beam_on', BP_BEAM_SPOT_CONSTRAINT='true', BP_FREE_LOSS_FIT='false', BP_LOCATION_SCAN_SETUPS='')
        on = self.manifest('beam_on')
        self.assertEqual(on['controls']['BP_BEAM_SPOT_CONSTRAINT'], '1')
        with patch.dict(os.environ, on['controls'], clear=True):
            exec(assignments, {'fit': fit, 'os': os})
        self.assertTrue(fit.BeamSpotConstraint)
        for name in ('BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE', 'BP_BEAM_FREE_LOSS_BACKWARD_FILTER'):
            with self.assertRaisesRegex(ValueError, 'was removed'):
                self.prepare(OUTPUT_TUPLEPATH='retired_beam', **{name: '0'})

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
        env = dict(self.env, STAGES='trk,calodigi,rec,breakpoint',
                   SEED_FIRST='11', SEED_LAST='14', DRY_RUN='0')
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
            self.prepare(STAGES='trk,calodigi,rec,breakpoint', SEED_FIRST='13', SEED_LAST='14')
        self.assertFalse((self.repo/'outputs').exists())

    def test_breakpoint_only_skips_missing_tracker(self):
        self.prepare(STAGES='breakpoint', SEED_LAST='13')
        self.assertEqual(self.manifest()['stages'], ['breakpoint'])
        self.assertFalse((self.repo/'outputs/runcards/e--2.0-85-13').exists())

    def test_generated_sim_needs_no_external_sim(self):
        self.prepare(STAGES='sim,trk,calodigi,rec,breakpoint', SEED_FIRST='13', SEED_LAST='13')
        self.assertTrue((self.repo/'outputs/runcards/e--2.0-85-13/job.json').is_file())

    def test_submission_wrapper_constraints_default_110_and_explicit_override(self):
        controls = ('BP_BEAM_SPOT_CONSTRAINT', 'BP_CHARGED_ECAL_MEASUREMENT',
                    'BP_NEUTRAL_ECAL_MEASUREMENT')
        assignments = ('fit.BeamSpotConstraint =', 'fit.ChargedEcalMeasurement =',
                       'fit.NeutralEcalMeasurement =')
        for label, override, expected in [('default', {}, '110'),
                                         ('enable_111', dict(zip(controls, '111')), '111'),
                                         ('override', dict(zip(controls, '101'), BP_LOCATION_SCAN_SETUPS=''), '101')]:
            env = dict(self.env, OUTPUT_TUPLEPATH='constraints_' + label, **override)
            result = batch.subprocess.run(['bash', str(self.repo/'subbreakpointjobs.sh')],
                                          env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            carddir = self.repo/('constraints_' + label)/'runcards/barrel-12'
            job = json.loads((carddir/'job.json').read_text())
            self.assertEqual(''.join(job['controls'][k] for k in controls), expected)
            card = Path(job['cards']['breakpoint']).read_text()
            template = (self.repo/'Reconstruction/RecBreakpoint/options/run_breakpoint.py').read_text()
            fit = types.SimpleNamespace()
            with patch.dict(os.environ, dict(zip(controls, '000')), clear=True):
                exec(card[:-len(template)] + '\n' + '\n'.join(
                    line for line in template.splitlines() if line.startswith(assignments)),
                    {'fit': fit, 'os': os})
            self.assertEqual([fit.BeamSpotConstraint, fit.ChargedEcalMeasurement,
                              fit.NeutralEcalMeasurement], [v == '1' for v in expected])
            self.assertFalse((carddir/'submitted.json').exists())

    def test_submission_wrapper_refit_default_and_full_chain_overrides(self):
        for stages in (None, ','.join(batch.ORDER), ','.join(batch.ORDER[1:])):
            with self.subTest(stages=stages):
                output = 'wrapper_default' if stages is None else 'wrapper_' + stages.replace(',', '_')
                env = dict(self.env, PATH=os.environ.get('PATH', '/usr/bin:/bin'),
                           SAMPLE_REGION='barrel', OUTPUT_TUPLEPATH=output)
                if stages is None:
                    del env['STAGES']
                else:
                    env['STAGES'] = stages
                result = batch.subprocess.run(
                    ['bash', str(self.repo/'subbreakpointjobs.sh')], env=env,
                    capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                carddir = self.repo/output/'runcards/barrel-12'
                job = json.loads((carddir/'job.json').read_text())
                self.assertEqual(job['stages'], ['breakpoint'] if stages is None else stages.split(','))
                self.assertEqual(job['controls']['BP_CHARGED_ECAL_ENERGY_MODE'], 'ElectronPhoton')
                self.assertEqual(job['controls']['BP_EXPERIMENTAL_DELAYED_NEUTRAL'], '1')
                self.assertFalse((carddir/'submitted.json').exists())
                for stage, predecessor in zip(batch.ORDER[1:], batch.ORDER[:-1]):
                    if stage not in job['stages']: continue
                    card = Path(job['cards'][stage]).read_text()
                    self.assertIn(repr(job['files'][predecessor]), card)
                    expected = self.repo/(output if predecessor in job['stages'] else 'inputs')
                    self.assertEqual(Path(job['files'][predecessor]).parent, expected)

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

    def test_submission_shell_group_override_and_validation(self):
        env = dict(self.env, SUBMIT_GROUP='higgs')
        result = batch.subprocess.run(['bash', str(self.repo/'subbreakpointjobs.sh')],
                                      env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('-g higgs ', result.stdout)
        carddir = self.repo/'outputs/runcards/barrel-12'
        self.assertFalse((carddir/'submitted.json').exists())
        for group in ('', '-cms', 'cms higgs', 'cms;echo'):
            with patch.dict(os.environ, {'SUBMIT_GROUP': group}, clear=True):
                with self.assertRaisesRegex(ValueError, 'SUBMIT_GROUP'):
                    batch.submit_jobs([], True)

    def test_submit_prepared_preserves_cards_and_blocks_duplicates(self):
        self.prepare()
        job=self.manifest()
        hashes=dict(job['checksums'])
        with patch.dict(os.environ,dict(self.env, SUBMIT_GROUP='higgs'),clear=True), contextlib.redirect_stdout(io.StringIO()) as stdout:
            batch.submit_prepared('outputs')  # dry run: no scheduler
        self.assertIn('-g higgs ', stdout.getvalue())
        self.assertEqual(self.manifest()['checksums'],hashes)
        marker=Path(job['cards']['trk']).parent/'submitted.json'; marker.write_text('{}')
        with patch.dict(os.environ,self.env,clear=True), self.assertRaises(ValueError):
            batch.submit_prepared('outputs')

    def cleanup_fixture(self, stages='trk,calodigi,rec'):
        self.prepare(STAGES=stages)
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        paths={stage:Path(job['files'][stage]) for stage in ('trk','calodigi')}
        identities={}
        for stage in ('trk','calodigi'):
            if stage in job['stages']:
                paths[stage].write_bytes(b'job-owned '+stage.encode())
                identities[stage]=batch.file_identity(paths[stage])
        return job,manifest,paths,identities

    def test_cleanup_only_owned_complete_tracker(self):
        job,manifest,paths,identities=self.cleanup_fixture()
        result=batch.cleanup_intermediate(job,manifest,'trk',identities['trk'],True)
        self.assertEqual(result['status'],'removed');self.assertFalse(paths['trk'].exists())
        self.assertTrue(paths['calodigi'].exists())
        self.assertTrue(Path(job['files']['sim']).exists())

    def test_cleanup_only_owned_complete_calodigi(self):
        job,manifest,paths,identities=self.cleanup_fixture()
        result=batch.cleanup_intermediate(job,manifest,'calodigi',identities['calodigi'],True)
        self.assertEqual(result['status'],'removed');self.assertFalse(paths['calodigi'].exists())
        self.assertTrue(paths['trk'].exists())

    def test_cleanup_retains_unverified_output(self):
        job,manifest,paths,identities=self.cleanup_fixture()
        for stage in ('trk','calodigi'):
            result=batch.cleanup_intermediate(job,manifest,stage,identities[stage],False)
            self.assertEqual(result['status'],'retained_unverified_output')
            self.assertTrue(paths[stage].exists())

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
        for defect in ('zombie', 'recovered', 'empty', 'missing_branch',
                       'missing_constraint_branch', 'missing_truth_entry_branch'):
            with self.subTest(defect=defect):
                tree = MagicMock()
                tree.GetEntries.return_value = 0 if defect == 'empty' else 2
                tree.GetBranch.return_value = None if defect == 'missing_branch' else object()
                if defect == 'missing_constraint_branch':
                    tree.GetBranch.side_effect = lambda name: None if name == 'neutral_ecal_free_loss_rts_pt' else object()
                if defect == 'missing_truth_entry_branch':
                    tree.GetBranch.side_effect = lambda name: None if name == 'truth_ecal_entry_status' else object()
                file = MagicMock()
                file.IsZombie.return_value = defect == 'zombie'
                file.TestBit.return_value = defect == 'recovered'
                file.Get.return_value = tree
                root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path: file, kRecovered=1))
                with patch.dict('sys.modules', ROOT=root), self.assertRaises(RuntimeError):
                    batch.verify(Path('fixture.root'), 'breakpoint')

    def test_missing_durable_rec_input_blocks_calodigi_cleanup(self):
        tree = MagicMock()
        tree.GetEntries.return_value = 2
        tree.GetBranch.side_effect = lambda name: None if name == 'TPCTrackerHitAss' else object()
        file = MagicMock()
        file.IsZombie.return_value = False
        file.TestBit.return_value = False
        file.Get.return_value = tree
        root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path: file, kRecovered=1))
        with patch.dict('sys.modules', ROOT=root), self.assertRaisesRegex(RuntimeError, 'TPCTrackerHitAss'):
            batch.verify(Path('fixture.root'), 'rec')

    def test_cleanup_retains_external_input(self):
        job,manifest,paths,identities=self.cleanup_fixture('rec,breakpoint')
        result=batch.cleanup_intermediate(job,manifest,'calodigi',None,True)
        self.assertEqual(result['status'],'retained_external');self.assertTrue(paths['calodigi'].exists())

    def test_missing_pre_pfo_blocks_rec_cleanup_only_when_requested(self):
        tree = MagicMock()
        tree.GetEntries.return_value = 2
        tree.GetBranch.side_effect = lambda name: None if name == 'CyberPFOPreReclustering' else object()
        file = MagicMock()
        file.IsZombie.return_value = False
        file.TestBit.return_value = False
        file.Get.return_value = tree
        root = types.SimpleNamespace(TFile=types.SimpleNamespace(Open=lambda path: file, kRecovered=1))
        with patch.dict('sys.modules', ROOT=root):
            self.assertTrue(batch.verify(Path('fixture.root'), 'rec', 'FinalPFO'))
            with self.assertRaisesRegex(RuntimeError, 'CyberPFOPreReclustering'):
                batch.verify(Path('fixture.root'), 'rec', 'PreReclustering')

    def test_cleanup_retains_trk_only_output(self):
        job,manifest,paths,identities=self.cleanup_fixture('trk')
        result=batch.cleanup_intermediate(job,manifest,'trk',identities['trk'],True)
        self.assertEqual(result['status'],'retained_no_downstream');self.assertTrue(paths['trk'].exists())

    def test_cleanup_refuses_changed_or_redirected_tracker(self):
        job,manifest,paths,identities=self.cleanup_fixture()
        for stage in ('trk','calodigi'):
            intermediate=paths[stage]
            intermediate.write_bytes(b'changed after production')
            with self.assertRaises(RuntimeError):
                batch.cleanup_intermediate(job,manifest,stage,identities[stage],True)
            intermediate.unlink()
            shared=self.repo/f'inputs/{stage}-e--2.0-85-12.root'
            intermediate.symlink_to(shared)
            with self.assertRaises(RuntimeError):
                batch.cleanup_intermediate(job,manifest,stage,batch.file_identity(intermediate),True)
            self.assertTrue(shared.exists())

    def test_worker_cleans_each_intermediate_after_its_consumer(self):
        self.prepare(STAGES='trk,calodigi,rec,breakpoint')
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        observed=[]
        def execute(command, **kwargs):
            stage=Path(command[-1]).stem
            self.assertEqual(command, ['/project/environment/gaudirun.py', job['cards'][stage]])
            observed.append((stage, Path(job['files']['trk']).exists(),
                             Path(job['files']['calodigi']).exists()))
            Path(job['files'][stage]).write_bytes(b'produced '+stage.encode())
        with patch.object(Path,'cwd',return_value=self.repo), patch.object(batch.subprocess,'run',side_effect=execute), \
             patch.object(batch,'verify',return_value=True), \
             patch.object(batch.shutil,'which',return_value='/project/environment/gaudirun.py') as which:
            batch.run(manifest)
        which.assert_called_once_with('gaudirun.py')
        self.assertEqual(observed, [('trk',False,False), ('calodigi',True,False),
                                    ('rec',False,True), ('breakpoint',False,False)])
        self.assertFalse(Path(job['files']['trk']).exists())
        self.assertFalse(Path(job['files']['calodigi']).exists())
        self.assertTrue(Path(job['files']['sim']).exists())
        self.assertTrue(Path(job['files']['rec']).exists())
        self.assertTrue(Path(job['files']['breakpoint']).exists())
        completed=json.loads((manifest.parent/'completed.json').read_text())
        self.assertEqual(completed['intermediate_cleanup']['trk']['status'],'removed')
        self.assertEqual(completed['intermediate_cleanup']['calodigi']['status'],'removed')

    def test_rec_failure_retains_calodigi_and_sim(self):
        self.prepare(STAGES='trk,calodigi,rec,breakpoint')
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        def execute(command, **kwargs):
            stage=Path(command[-1]).stem
            if stage == 'rec': raise batch.subprocess.CalledProcessError(1,command)
            Path(job['files'][stage]).write_bytes(b'produced '+stage.encode())
        with patch.object(Path,'cwd',return_value=self.repo), patch.object(batch.subprocess,'run',side_effect=execute), \
             patch.object(batch,'verify',return_value=True), \
             patch.object(batch.shutil,'which',return_value='/project/environment/gaudirun.py'):
            with self.assertRaises(batch.subprocess.CalledProcessError): batch.run(manifest)
        self.assertFalse(Path(job['files']['trk']).exists())
        self.assertTrue(Path(job['files']['calodigi']).exists())
        self.assertTrue(Path(job['files']['sim']).exists())
        self.assertFalse((manifest.parent/'completed.json').exists())

    def test_worker_failure_does_not_cleanup_tracker(self):
        self.prepare()
        job=self.manifest()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        def execute(command, **kwargs):
            if Path(command[-1]).name=='trk.py':
                Path(job['files']['trk']).write_bytes(b'produced tracker')
            else:
                raise batch.subprocess.CalledProcessError(1,command)
        with patch.object(Path,'cwd',return_value=self.repo), patch.object(batch.subprocess,'run',side_effect=execute), \
             patch.object(batch,'verify',return_value=False), patch.object(batch,'cleanup_intermediate') as cleanup, \
             patch.object(batch.shutil,'which',return_value='/project/environment/gaudirun.py'):
            with self.assertRaises(batch.subprocess.CalledProcessError):batch.run(manifest)
            cleanup.assert_not_called()
        self.assertTrue(Path(job['files']['trk']).exists())
        self.assertFalse((manifest.parent/'completed.json').exists())

    def test_worker_missing_project_launcher_does_not_start(self):
        self.prepare()
        manifest=self.repo/'outputs/runcards/e--2.0-85-12/job.json'
        with patch.object(Path,'cwd',return_value=self.repo), \
             patch.object(batch.shutil,'which',return_value=None), \
             patch.object(batch.subprocess,'run') as execute:
            with self.assertRaisesRegex(ValueError, 'source project setup.sh'):
                batch.run(manifest)
            execute.assert_not_called()
        self.assertFalse((manifest.parent/'started.json').exists())


if __name__ == '__main__': unittest.main()
