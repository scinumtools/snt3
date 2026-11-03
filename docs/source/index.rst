Scientific Numerical Tools v3
=============================

`SciNumTools v3 <https://github.com/scinumtools/snt3>`_ provides a modular C++
core for physical quantities and structured scientific parameters. PUEL
describes quantities and units; DIPL defines typed parameters, constraints,
and relationships. The same definitions are available through Python, the
command line, CMake, and service interfaces. DIP can also save evaluated
configurations and experiment records as DIPH5.

Start with the :doc:`quickstart` or :doc:`introduction`. For scientific-code
integration, see the :doc:`initiative`; for checked parameter changes by
agents, see the :doc:`agent-workflow` and :doc:`MCP guide <integrations/mcp>`.

The official SNT3 benchmarking project is
`scinumtools/snt-bench <https://github.com/scinumtools/snt-bench>`_.
For measured results and the changes behind them, see
:doc:`performance/optimisation-progress`.

.. toctree::
   :maxdepth: 2
   :caption: Getting Started
   
   introduction
   installation
   dependencies
   quickstart
   initiative
   agent-workflow

.. toctree::
   :maxdepth: 2
   :caption: Core Concepts
   
   puel/index
   dipl/index

.. toctree::
   :maxdepth: 2
   :caption: Modules

   modules/index
   modules/core/index
   modules/val/index
   modules/exs/index
   modules/puq/index
   modules/dip/index
   modules/mat/index
   modules/api/index

.. toctree::
   :maxdepth: 2
   :caption: Interfaces and Integrations

   integrations/python
   integrations/c
   integrations/web
   integrations/cli
   integrations/devtools
   integrations/hub
   integrations/viewer
   integrations/cmake
   integrations/rest
   integrations/mcp
   integrations/docker

.. toctree::
   :maxdepth: 2
   :caption: Resources

   examples/index
   performance/optimisation-progress
   api/index

Previous version
----------------

SciNumTools v2 established the Python implementation. Version 3 moves its
scientific concepts into a modular C++ foundation for use across languages and
applications. The original `SciNumTools2 documentation
<https://scinumtools.github.io/snt2/>`_ remains available.
