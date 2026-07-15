# Removal of the experimental intermediate-support feature

Date: 2026-07-14

The user requested complete removal of the rejected experimental control that
split a configurable fraction of the CEPC g3 Bethe-Heitler weight into an
additional 5--10% loss component.

Removed from active code and tooling:

- the conditional means/variances and mixture-insertion helper in
  `BetheHeitlerSplitter.cpp`;
- the splitter constructor argument and stored fraction;
- the `BHIntermediateSupportFraction` Gaudi property and all three algorithm
  call-site connections;
- the `GSF_BH_INTERMEDIATE_SUPPORT_FRACTION` steering variable;
- the sample-runner command-line option; and
- the dedicated `compare_intermediate_support_scan.py` analysis script.

The standard five-component `CEPC2GeV85StepConditioned` mixture is unchanged.
Earlier experiment records are retained as immutable provenance under
`agents_record/`; their references to the removed experiment do not describe
an available feature.

Validation:

- a case-insensitive active-tree search found no remaining property, member,
  helper, coefficient-table, environment-variable, CLI-option, or script-name
  symbol associated with the feature;
- both modified Python steering files pass `python3 -m py_compile`, and the
  sample runner's `--help` no longer advertises the removed option;
- `RecGsfTracking` rebuilt and the configured CEPCSW tree installed
  successfully (only pre-existing compiler and ROOT installation warnings);
- a comprehensive-component default-24 rerun of light event 284/1 completed
  successfully with all 232 hits, zero reverse rejection, 24 final reverse
  components, and GSF IP pT 2.0275 GeV. The first forward split produced the
  standard five children, confirming that the sixth experimental child is no
  longer constructed. The log is
  `/tmp/gsf-intermediate-support-removed-284-1/seed-284.log`.
