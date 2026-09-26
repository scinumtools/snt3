DIP C++ API
===========

DIPL parsing, environments, cursors, nodes, registries, and expression
solvers. Lower-level types are included for applications extending DIP.

For everyday node access, use Cursor for known paths and Select for discovery;
see :doc:`Basic Usage <../modules/dip/basic-usage>`. Existing request methods
remain available in the reference, including ``request_value()``,
``request_group()``, ``request_map()``, ``request_list()``, and
``request_node_data()``. ``request_group()`` retains relative result paths,
any-tag filtering, and its exception when no nodes match.

.. toctree::
   :maxdepth: 1

   cpp/dip/parsing
   cpp/dip/nodes
   cpp/dip/registries
   cpp/dip/solvers
