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
from importlib.util import find_spec

# Use the installed package (as in CI), or a compiled in-tree package for
# local builds. Source-only wrappers cannot expose the pybind11 API.
_docs_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
_dipl_highlight = os.path.join(_docs_root, 'docs', 'dipl', 'highlight', 'pygments')
sys.path.insert(0, _dipl_highlight)
_python_builds = [os.path.join(_docs_root, 'build', 'python')]
_python_builds.extend(glob.glob(os.path.join(_docs_root, 'build', '*', 'python')))
if find_spec('scinumtools3') is None:
   _extension_suffix = sysconfig.get_config_var('EXT_SUFFIX') or ''
   for _python_build in _python_builds:
      if os.path.isfile(os.path.join(_python_build, 'scinumtools3', '_snt' + _extension_suffix)):
         sys.path.insert(0, _python_build)
         break

try:
   from scinumtools3 import _snt
except ImportError as exc:
   raise RuntimeError(
      'Python API documentation requires the compiled scinumtools3 package. '
      'Run python -m pip install ".[docs]" or build the Python bindings '
      'with the same Python interpreter used by Sphinx.'
   ) from exc

extensions = [
   'sphinx.ext.autodoc',
]

from sphinx.highlighting import lexers
from syntax_lexer import SyntaxLexer

lexers['dipl'] = SyntaxLexer()

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_css_files = ['dipl.css']
