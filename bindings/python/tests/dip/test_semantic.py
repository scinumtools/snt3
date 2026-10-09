from scinumtools3.dip import (
    PreviewOverride,
    PreviewOverrideKind,
    describe,
    inspect_value,
    list_descriptions,
    open_artifact,
    override_contract,
    OverrideTargetKind,
    preview,
)


def test_describe_list_and_preview_share_evaluated_paths(tmp_path):
    source = tmp_path / "model.dip"
    source.write_text(
        'physics\n'
        '  speed float = 2 m/s\n'
        '    !tags ["runtime"]\n'
        '    ?descr "Flow speed"\n'
        '  samples int[3] = [1, 2, 3]\n'
    )
    env = open_artifact(source)
    speed = describe(env, "physics.speed")
    assert speed.kind == "value"
    assert speed.metadata.description == "Flow speed"
    assert speed.value_text == "2"
    assert speed.units is not None
    assert speed.source_text_available
    contract = override_contract(env, "physics.speed")
    assert contract.kind == OverrideTargetKind.ExistingValue
    assert contract.resolved_path == "physics.speed"
    assert contract.units is not None
    assert override_contract(env, "missing").reason == "unknown_value"

    selected = list_descriptions(env, "?physics.", tags_all=["runtime"], limit=1)
    assert selected.total == 1
    assert [item.path for item in selected.items] == ["physics.speed"]
    assert selected.items[0].value_text is None
    assert env.select_paths("?physics.", tags_all=["runtime"]) == ["physics.speed"]
    assert describe(env, "physics.samples", 2).value_unavailable_reason == "omitted_by_limit"

    valid = preview(source, [PreviewOverride(PreviewOverrideKind.Text, "physics.speed = 3 m/s")])
    assert valid.baseline_valid and valid.candidate_valid
    assert valid.accepted_override_targets == ["physics.speed"]
    assert [item.path for item in valid.comparison.differences] == ["physics.speed"]
    invalid = preview(source, [PreviewOverride(PreviewOverrideKind.Text, "missing = 1")])
    assert invalid.baseline_valid and not invalid.candidate_valid
    assert invalid.candidate_diagnostics[0].code
    assert describe(env, "physics.speed").value_text == "2"

    snapshot = tmp_path / "model.diph5"
    env.save(snapshot)
    saved = describe(open_artifact(snapshot), "physics.speed")
    inspected_saved = inspect_value(open_artifact(snapshot), "physics.speed")
    assert saved.value_text == "2"
    assert not saved.source_text_available
    assert saved.declaration.source == inspected_saved.declaration_location.source
    assert saved.declaration.line == inspected_saved.declaration_location.line
    assert override_contract(open_artifact(snapshot), "physics.speed").snapshot_input
