# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
# Keep Sphinx and the compiled bindings on the same Python interpreter.
# Override PYTHON to use another environment.
PYTHON       ?= $(if $(wildcard ../.venv/bin/python),../.venv/bin/python,python3)
SPHINXBUILD   ?= $(PYTHON) -m sphinx
SOURCEDIR     = source
BUILDDIR      = build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile doxygen

doxygen:
	doxygen Doxyfile

html: doxygen
# Doxygen rewrites the C++ symbol data. Rebuild Sphinx's environment too:
# cached parent-symbol references can become invalid when declarations move.
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" -E $(SPHINXOPTS) $(O)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
