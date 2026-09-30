from scinumtools3.dip import (
    ComparisonOptions,
    ComparisonScope,
    DIP,
    compare,
    compare_diph5,
    render_comparison,
)
from scinumtools3.api.dip import DIPCompare


def parsed(code):
    parser = DIP()
    parser.add_string(code)
    return parser.parse()


def test_comparison_scopes_and_files(tmp_path):
    before = parsed('value int = 1\n  ?descr "old"\narray int[3] = [1,2,3]\n')
    after = parsed('value int = 1\n  ?descr "new"\narray int[3] = [1,9,3]\n')
    effective = compare(before, after)
    assert (effective.added, effective.removed, effective.changed) == (0, 0, 1)
    assert effective.differences[0].example_indices == [1]
    assert "1 elements differ" in render_comparison(effective)

    options = ComparisonOptions()
    options.scope = ComparisonScope.Full
    assert compare(before, after, options).changed == 2

    first = tmp_path / "before.diph5"
    second = tmp_path / "after.diph5"
    before.save(first)
    after.save(second)
    assert compare_diph5(first, second).changed == 1
    command = DIPCompare(first, second)
    command.set_options(options)
    assert command.compare().changed >= 2
    assert "DIPH5 comparison" in command.execute()
