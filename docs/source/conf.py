# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'scinumtools3'
copyright = '2025, Ondrej Pego Jaura'
author = 'Ondrej Pego Jaura'
release = '0.1a'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

import os
import sys
import glob
import sysconfig

# Prefer the in-tree build of the extension, then fall back to the Python
# wrapper source. This lets autodoc inspect real pybind11 classes after the
# normal build while keeping documentation builds possible before compilation.
_docs_root = os.path.abspath('../..')
_python_builds = [os.path.join(_docs_root, 'build', 'python')]
_python_builds.extend(glob.glob(os.path.join(_docs_root, 'build', '*', 'python')))
for _python_build in reversed(_python_builds):
   sys.path.insert(0, _python_build)
sys.path.insert(1, os.path.join(_docs_root, 'bindings', 'python', 'src'))

extensions = [
   'sphinx.ext.autodoc',
]

_extension_suffix = sysconfig.get_config_var('EXT_SUFFIX') or ''
_extension_candidates = [
   candidate
   for build_path in _python_builds
   for candidate in glob.glob(os.path.join(build_path, 'scinumtools3', '_snt*' + _extension_suffix))
]
if not _extension_candidates:
   autodoc_mock_imports = ['scinumtools3._snt']

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
