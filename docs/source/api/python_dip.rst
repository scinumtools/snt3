DIP Python API
==============

DIPL parsing, evaluation environments, tree traversal, and parameter nodes.
The inspection functions expose evaluated value snapshots, ordered table
metadata, in-memory slices, artifact loading, and structured DIP diagnostics.

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
