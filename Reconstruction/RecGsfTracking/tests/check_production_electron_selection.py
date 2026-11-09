#!/usr/bin/env python3
"""Check selection provenance against actual published tracks and fit lineage."""
import argparse
import math

import ROOT


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tuple")
    parser.add_argument("--events", type=int, nargs="+")
    parser.add_argument("--expect-selected", type=int, choices=(1, 2))
    args = parser.parse_args()
    source = ROOT.TFile.Open(args.tuple, "READ")
    assert source and not source.IsZombie(), args.tuple
    tree = source.Get("gsf_tuple")
    assert tree, "Missing gsf_tuple"
    processed = []
    for event in range(tree.GetEntries()):
        tree.GetEntry(event)
        status = int(tree.production_electron_selection_status)
        outputs = list(tree.gsf_input_track_indices)
        lineage = set(tree.lineage_node_input_track_index)
        components = list(tree.final_mixture_component_input_track_index)
        if status == -5:
            assert not outputs and not lineage and not components
            continue
        processed.append(event)
        selected = {i for i in tree.production_electron_track_indices if i >= 0}
        if args.expect_selected is not None:
            assert status in (1, 2) and len(selected) == args.expect_selected
        if status in (1, 2):
            assert len(selected) == (2 if status == 1 else 1)
            assert list(tree.production_electron_candidate_counts) == [1, 1]
        if status != 0:
            assert set(outputs) <= selected and lineage <= selected
            assert set(components) <= selected
            if status < 0:
                assert not outputs and not lineage and not components
        assert len(outputs) == len(set(outputs))
        fields = (tree.final_mixture_component_output_track_index,
                  tree.final_mixture_component_weight,
                  tree.final_mixture_component_kappa)
        assert all(len(field) == len(components) for field in fields)
        lineage_inputs = tree.lineage_node_input_track_index
        lineage_outputs = tree.lineage_node_output_track_index
        assert len(lineage_inputs) == len(lineage_outputs)
        for index, output in zip(lineage_inputs, lineage_outputs):
            assert output == -1 or (0 <= output < len(outputs) and outputs[output] == index)
        weights = {}
        for index, output, weight, kappa in zip(
            components, tree.final_mixture_component_output_track_index,
            tree.final_mixture_component_weight, tree.final_mixture_component_kappa
        ):
            assert 0 <= output < len(outputs) and outputs[output] == index
            assert math.isfinite(weight) and weight > 0 and math.isfinite(kappa)
            weights[output] = weights.get(output, 0.0) + weight
        assert all(math.isclose(w, 1.0, abs_tol=1e-10) for w in weights.values())
        print(f"event={event} status={status} selected={sorted(selected)} "
              f"published={outputs} lineage_tracks={sorted(lineage)}")
    if args.events is not None:
        assert processed == args.events, (processed, args.events)
    source.Close()


if __name__ == "__main__":
    main()
