DIP Python API
==============

DIPL parsing, evaluation environments, tree traversal, and parameter nodes.
The inspection functions expose evaluated value snapshots, ordered table
metadata, in-memory slices, artifact loading, optional dependency graphs, and
structured DIP diagnostics. See :doc:`the DIP inspection guide
<../modules/dip/inspection>` for graph recording and interpretation.
The adapter classes and runners are documented in :doc:`the adapter guide
<../modules/dip/adapters>`.

For everyday node access, use Cursor for known paths and Select for discovery;
see :doc:`the Python guide <../integrations/python>`. The request helpers below
remain available for existing callers. ``request_value()`` returns a value with
optional unit conversion or NumPy output. ``request_group()`` returns node
snapshots with relative paths, matches any supplied tag, and raises an exception
when no nodes match.

.. automodule:: scinumtools3.dip
   :members:
   :undoc-members:

.. autoclass:: scinumtools3.dip.DIP
   :members:

.. autoclass:: scinumtools3.dip.Environment
   :members:

.. autoclass:: scinumtools3.dip.Cursor
   :members:

.. autoclass:: scinumtools3.dip.ValueNode
   :members:

.. autoclass:: scinumtools3.dip.ValueNodeData
   :members:

.. _python-dip-adapters:

Application adapters
--------------------

.. autoclass:: scinumtools3.dip.Adapter
   :members:

.. autoclass:: scinumtools3.dip.AdapterContext
   :members:

.. autoclass:: scinumtools3.dip.OutputPlan
   :members:

.. autoclass:: scinumtools3.dip.OutputMapping
   :members:

.. autofunction:: scinumtools3.dip.resolve_output_plan

.. autofunction:: scinumtools3.dip.run_adapter

.. autofunction:: scinumtools3.dip.run_adapter_project

.. autofunction:: scinumtools3.dip.run_adapter_snapshot
