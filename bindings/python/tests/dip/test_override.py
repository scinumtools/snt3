import pytest

from scinumtools3.dip import DIP, Environment


def test_override_precedes_dependencies_and_ordinary_modifications(tmp_path):
    # The override replaces a constant before dependency evaluation and wins over later assignments.
    parser = DIP()
    parser.add_string(
        '$override\n'
        '  radius = 20 cm\n'
        'radius float = 10 cm\n'
        '  !constant\n'
        'radius = 30 cm\n'
        'diameter float = ({?radius} * 2) cm\n'
    )
    env = parser.parse()
    assert env['radius'].value == 20
    assert env['diameter'].value == 40
    assert env.select('?radius')[0].override is True
    assert env['radius'].provenance.override_line == 2

    # A snapshot preserves both the effective flag and the source of the replacement.
    file = tmp_path / 'override.diph5'
    env.save(file)
    loaded = Environment()
    loaded.load(file)
    assert loaded.select('?radius')[0].override is True
    assert loaded['radius'].provenance.override_code == '  radius = 20 cm'


def test_host_override_sources(tmp_path):
    # Strings and files register the same kind of overrides before model evaluation.
    parser = DIP()
    parser.add_override_string('first = 5')
    file = tmp_path / 'overrides.dip'
    file.write_text('second = 6')
    parser.add_override_file(file)
    parser.add_string('first int = 1\nsecond int = 2')
    env = parser.parse()
    assert env['first'].value == 5
    assert env['second'].value == 6
    assert env['first'].provenance.override_source is not None


def test_duplicate_and_missing_override():
    # Even identical duplicates fail; a target must actually be defined by the model.
    parser = DIP()
    parser.add_override_string('value = 2')
    with pytest.raises(RuntimeError, match='Duplicate override'):
        parser.add_override_string('value = 2')

    parser = DIP()
    parser.add_string('$override\n  missing = 2\nvalue int = 1')
    with pytest.raises(RuntimeError, match='Unresolved override'):
        parser.parse()


def test_ignored_modification_preserves_hierarchy():
    # Ignoring a value assignment must still attach its children to a, not the preceding b.
    parser = DIP()
    parser.add_string('a int = 1\nb int = 2\na = 3\n  child int = 4')
    parser.add_override_string('a = 9')
    env = parser.parse()
    assert [(n.name, n.value) for n in env.select('?')] == [('a', 9), ('b', 2), ('a.child', 4)]


def test_ignored_declaration_still_checks_type():
    # Suppressing a replacement value must not hide an incompatible type declaration.
    parser = DIP()
    parser.add_string('a int = 1\na str = "bad"')
    parser.add_override_string('a = 9')
    with pytest.raises(RuntimeError, match='Type mismatch'):
        parser.parse()


@pytest.mark.parametrize('original', ['[1,2]', '{?source}[1:2]'])
@pytest.mark.parametrize('replacement, expected', [('[7,8]', [7,8]), ('{?source}[0:1]', [1,2])])
def test_override_owns_its_expression(original, replacement, expected):
    # Only the replacement expression supplies a slice; the declared array shape stays fixed.
    parser = DIP()
    parser.add_string(f'source int[3] = [1,2,3]\na int[2] = {original}')
    parser.add_override_string(f'a = {replacement}')
    env = parser.parse()
    assert env['a'].value == expected
    assert env['a'].shape == [2]


@pytest.mark.parametrize('body', ['a = 2\nb int = 3', 'a = 2\na = 3'])
def test_rejected_registration_does_not_change_model(body):
    # A rejected body must leave no entries behind, so registering a again succeeds.
    parser = DIP()
    parser.add_string('a int = 1')
    with pytest.raises(RuntimeError):
        parser.add_override_string(body)
    parser.add_override_string('a = 4')
    env = parser.parse()
    assert env['a'].value == 4
    assert env['a'].provenance.override_source is not None


@pytest.mark.parametrize('host_schema', [False, True])
def test_schema_cannot_declare_override(host_schema):
    # Both schema entry points must reject nested overrides with a normal diagnostic.
    parser = DIP()
    with pytest.raises(RuntimeError, match='Invalid override region'):
        if host_schema:
            parser.add_schema_string('settings', 'a int = 1\n$override\n  group.a = 3')
        else:
            parser.add_string('$schema settings\n  a int = 1\n  $override\n    group.a = 3\ngroup : settings')
            parser.parse()


def test_conditions_use_overridden_values_and_dependencies():
    # A changed value propagates through a dependency and activates an existing definition.
    parser = DIP()
    parser.add_string('enabled bool = false\nactive bool = {?enabled}\n@if {?active}\n  value int = 1\n@end')
    parser.add_override_string('enabled = true\nvalue = 4')
    env = parser.parse()
    assert env['active'].value is True
    assert env.select('?value')[0].value == 4


def test_inactive_override_target_is_unresolved():
    # Overrides cannot create a target that the resulting condition excludes.
    parser = DIP()
    parser.add_string('enabled bool = true\n@if {?enabled}\n  value int = 1\n@end')
    parser.add_override_string('enabled = false\nvalue = 4')
    with pytest.raises(RuntimeError, match='Unresolved override'):
        parser.parse()


def test_override_preserves_declaration_units_and_metadata():
    # Convert the replacement to the declared metres while retaining descriptive properties.
    parser = DIP()
    parser.add_string('distance float = 1 m\n  !constant\n  !tags ["export"]\n  ?descr "Distance"')
    parser.add_override_string('distance = 200 cm')
    env = parser.parse()
    node = env.select('?distance')[0]
    assert node.value == 2
    assert node.tags == ['export']
    assert node.metadata.description == 'Distance'


@pytest.mark.parametrize('dtype', ['int32', 'float32'])
def test_override_preserves_numeric_type(dtype):
    # Evaluation and unit conversion must retain the declared 32-bit numeric type.
    from scinumtools3.core import DataType
    parser = DIP()
    parser.add_string(f'value {dtype} = 1 m')
    parser.add_override_string('value = 200 cm')
    node = parser.parse().select('?value')[0]
    assert node.value == 2
    assert node.dtype == (DataType.Integer32 if dtype == 'int32' else DataType.Float32)


def test_override_file_errors_and_provenance(tmp_path):
    # File errors leave registration unchanged; accepted files retain traceable provenance.
    file = tmp_path / 'overrides.dip'
    parser = DIP()
    with pytest.raises(RuntimeError, match='File not found'):
        parser.add_override_file(file)
    file.write_text('value = 2\nother int = 3')
    with pytest.raises(RuntimeError, match='Invalid override entry'):
        parser.add_override_file(file)
    file.write_text('value = 4')
    parser.add_override_file(file)
    with pytest.raises(RuntimeError, match='Duplicate override'):
        parser.add_override_string('value = 5')
    parser.add_string('value int = 1')
    env = parser.parse()
    assert env['value'].value == 4
    assert str(env['value'].provenance.override_source.path) == str(file)
    # Loading an evaluated snapshot must still identify the originating override file.
    snapshot = tmp_path / 'snapshot.diph5'
    env.save(snapshot)
    loaded = Environment()
    loaded.load(snapshot)
    assert str(loaded['value'].provenance.override_source.path) == str(file)


@pytest.mark.parametrize('declaration, replacement, message', [
    ('value int = 1\n  !options [1,2]', 'value = 3', 'Invalid option'),
    ('value int = 1\n  !condition ({.} > 0)', 'value = -1', 'Node does not satisfy its condition'),
    ('value str = "abc"\n  !format "[a-z]+"', 'value = "123"', 'format'),
])
@pytest.mark.parametrize('origin', ['inline', 'string', 'file'])
def test_override_constraint_error_identifies_both_inputs(tmp_path, declaration, replacement, message, origin):
    # Users tuning a model need the offending replacement as well as the declaration context.
    parser = DIP()
    parser.add_string(declaration)
    if origin == 'inline':
        parser.add_string('$override\n  ' + replacement)
    elif origin == 'string':
        parser.add_override_string(replacement)
    else:
        file = tmp_path / 'overrides.dip'
        file.write_text(replacement)
        parser.add_override_file(file)
    with pytest.raises(RuntimeError) as caught:
        parser.parse()
    diagnostic = str(caught.value)
    assert message.lower() in diagnostic.lower()
    assert 'Declaration: ' in diagnostic
    assert declaration.splitlines()[0] in diagnostic
    if origin == 'file':
        assert 'Override file: ' + str(file) in diagnostic
    location = next(line for line in diagnostic.splitlines() if line.strip().startswith('at:'))
    assert replacement in location
    assert (':2 |' if origin == 'inline' else ':1 |') in location


def test_schema_override_constraint_error_retains_schema_declaration():
    # A schema owns the constraint, while the override supplies the invalid value.
    parser = DIP()
    parser.add_schema_string('settings', 'value int = 1\n  !options [1,2]')
    parser.add_string('physics : settings')
    parser.add_override_string('physics.value = 3')
    with pytest.raises(RuntimeError) as caught:
        parser.parse()
    diagnostic = str(caught.value)
    assert 'Declaration: ' in diagnostic
    assert 'value int = 1' in diagnostic
    assert 'at:' in diagnostic and 'physics.value = 3' in diagnostic


@pytest.mark.parametrize('origin', ['inline', 'string', 'file'])
def test_nested_override_paths(tmp_path, origin):
    # Prefixes expand through the normal hierarchy rules without creating model nodes.
    parser = DIP()
    parser.add_string('simulation\n  steps int = 100\n  box_size float = 1 m\n  nested\n    enabled bool = false\nitems[a]\n  mass float = 1 kg\nparent int = 1\n  child int = 2')
    body = ('simulation\n  steps = 1024\n  box_size = 200 cm\n  nested\n    enabled = true\n'
            'items[a]\n  mass = 3 kg\nparent = 4\n  child = 5')
    if origin == 'inline':
        parser.add_string('$override\n' + '\n'.join('  ' + line for line in body.splitlines()))
    elif origin == 'string':
        parser.add_override_string(body)
    else:
        file = tmp_path / 'nested.dip'
        file.write_text(body)
        parser.add_override_file(file)
    env = parser.parse()
    assert [(node.name, node.value) for node in env.select('?')] == [
        ('simulation.steps', 1024), ('simulation.box_size', 2),
        ('simulation.nested.enabled', True), ('items[a].mass', 3),
        ('parent', 4), ('parent.child', 5),
    ]
    assert all(node.override for node in env.select('?'))
    assert env['simulation.steps'].provenance.override_code.strip() == 'steps = 1024'


@pytest.mark.parametrize('body', [
    'simulation\n  steps = 2\nsimulation.steps = 3',
    'simulation.steps = 3\nsimulation\n  steps = 2',
    'simulation\n  steps = 2\nsimulation\n  steps = 3',
])
def test_nested_override_duplicates_are_atomic(body):
    # Different spellings of the same full path are duplicates, not precedence rules.
    parser = DIP()
    with pytest.raises(RuntimeError, match='Duplicate override'):
        parser.add_override_string(body)
    parser.add_override_string('simulation.steps = 4')
    parser.add_string('simulation.steps int = 1')
    env = parser.parse()
    assert env['simulation.steps'].value == 4


@pytest.mark.parametrize('body', [
    'simulation\n  steps int = 2',
    'simulation : settings\n  steps = 2',
    'simulation map\n  steps = 2',
    'simulation\n  !tags ["export"]\n  steps = 2',
    'simulation\n  ?descr "Description"\n  steps = 2',
    'items[]\n  mass = 2',
    'simulation',
    'simulation\n    steps = 2',
    '  simulation.steps = 2',
])
def test_nested_override_rejects_structural_entries(body):
    # Indentation is shorthand for paths, never permission to declare or append nodes.
    parser = DIP()
    with pytest.raises(RuntimeError):
        parser.add_override_string(body)


def test_nested_override_missing_target_is_not_created():
    parser = DIP()
    parser.add_string('simulation.steps int = 1')
    parser.add_override_string('simulation\n  missing = 2')
    with pytest.raises(RuntimeError, match='Unresolved override'):
        parser.parse()


def test_nested_prefixes_compose_dotted_paths_and_list_indices():
    # Nested and dotted paths compose identically, including concrete list item selectors.
    parser = DIP()
    parser.add_string('root\n  items[]\n    nested.value int = 1\n  items[]\n    nested.value int = 2')
    parser.add_string('$override\n  root\n    # Select a concrete item, without appending one.\n    items[1]\n\n      nested.value = 8')
    env = parser.parse()
    assert [(n.name, n.value) for n in env.select('?')] == [
        ('root.items[0].nested.value', 1), ('root.items[1].nested.value', 8),
    ]


def test_nested_and_flat_overrides_conflict_across_origins():
    parser = DIP()
    parser.add_override_string('group\n  value = 2')
    parser.add_string('group.value int = 1\n$override\n  group.value = 3')
    with pytest.raises(RuntimeError, match='Duplicate override'):
        parser.parse()
