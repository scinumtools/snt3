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

.PHONY: help Makefile doxygen pdf-reference

pdf-reference:
	PYTHONPATH=../build/python:$$PYTHONPATH $(PYTHON) dipl/cheatsheet/build.py
	mkdir -p ../build
	cd .. && pandoc --defaults=docs/dipl/pandoc.yaml --output=build/dipl-specification.pdf
	cd .. && pandoc --defaults=docs/puel/pandoc.yaml --output=build/puel-specification.pdf
	cd .. && pandoc --defaults=docs/diph5/pandoc.yaml --output=build/diph5-specification.pdf
	cp ../build/dipl-specification.pdf dipl/dipl-specification.pdf
	cp ../build/puel-specification.pdf puel/puel-specification.pdf
	cp ../build/diph5-specification.pdf diph5/diph5-specification.pdf
	cp ../build/dipl-specification.pdf source/_static/dipl-specification.pdf
	cp ../build/puel-specification.pdf source/_static/puel-specification.pdf
	cp ../build/diph5-specification.pdf source/_static/diph5-specification.pdf

doxygen:
	doxygen Doxyfile

html: doxygen pdf-reference
# Doxygen rewrites the C++ symbol data. Rebuild Sphinx's environment too:
# cached parent-symbol references can become invalid when declarations move.
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" -E $(SPHINXOPTS) $(O)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
