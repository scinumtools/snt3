import pytest

from scinumtools3.dip import DIP
from scinumtools3.api.dip import DIPParse


@pytest.mark.parametrize("from_file", [False, True])
def test_schema_registration(tmp_path, from_file):
    body = ('speed float = 2 m/s\n  !tags ["export"]\n  ?descr "Flow speed"\n'
            'count int = {?base}\nchild\n  enabled bool = true\n')
    parser = DIP()
    if from_file:
        path = tmp_path / "settings.dipl"
        path.write_text(body)
        parser.add_schema_file("settings", path)
    else:
        parser.add_schema_string("settings", body)
    parser.add_string('base int = 4\nphysics : settings\n  speed = 3\nitems list : settings\nitems[]\n')
    env = parser.parse()
    assert env['physics.speed'].value == 3
    assert env['physics.count'].value == 4
    assert env['physics.child.enabled'].value is True
    assert env['items[0].speed'].value == 2
    node = env.select('?physics.speed')[0]
    assert node.tags == ['export']
    assert node.metadata.description == 'Flow speed'
    provenance = env['items[0].speed'].provenance
    assert provenance.source_line == 1
    import hashlib
    assert provenance.source.hash == hashlib.sha256(body.encode()).hexdigest()


@pytest.mark.parametrize('kind', ['schema_string', 'schema_file'])
def test_api_schema(tmp_path, kind):
    body = 'value int = 42'
    path = tmp_path / 'schema.dipl'
    path.write_text(body)
    command = DIPParse()
    command.argument_add(kind, ['settings', str(path) if kind == 'schema_file' else body])
    command.argument_add('string', ['physics : settings'])
    command.argument_request('physics.value')
    command.argument_value('integer')
    assert command.execute() == '42\n'
    with pytest.raises(RuntimeError):
        command.argument_load('environment.diph5')


def test_registration_errors(tmp_path):
    parser = DIP()
    for name, body in [('', 'v int'), ('bad name', 'v int'), ('empty', ''),
                       ('wrapper', '$schema inner\n  v int'), ('indent', '  v int'),
                       ('property', '!tags ["export"]')]:
        with pytest.raises(RuntimeError):
            parser.add_schema_string(name, body)
    with pytest.raises(RuntimeError):
        parser.add_schema_file('missing', tmp_path / 'missing.dipl')
    parser.add_schema_string('settings', 'value int')
    with pytest.raises(RuntimeError):
        parser.add_schema_string('settings', 'value int')
