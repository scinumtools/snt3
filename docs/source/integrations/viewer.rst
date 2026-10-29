Parameter Viewer
================

``snt view`` is an optional graphical browser and source editor for DIPL parameters.
It uses the evaluated DIP environment to show values, provenance, and recorded
dependencies. The viewer is part of the main ``snt`` executable; it does not
need a separate application binary.

.. container:: viewer-figures

   .. figure:: ../_static/snt-view/snt-view-node.png
      :alt: Parameter Viewer showing the parameter tree and inspector for a selected scientific input node.
      :width: 100%
      :target: ../_static/snt-view/snt-view-node.png

      Browse nodes and inspect values, provenance, and dependencies.

   .. figure:: ../_static/snt-view/snt-view-table.png
      :width: 100%
      :alt: Parameter Viewer displaying tabular parameter data.
      :target: ../_static/snt-view/snt-view-table.png

      Inspect array and table values in the data view.

   .. figure:: ../_static/snt-view/snt-view-source.png
      :width: 100%
      :alt: Parameter Viewer displaying a syntax-highlighted DIPL source with its source inspector.
      :target: ../_static/snt-view/snt-view-source.png

      View source text and navigate to the nodes it defines.

   .. figure:: ../_static/snt-view/snt-view-edit.png
      :width: 100%
      :alt: Parameter Viewer editing a DIPL source with line numbers and editing controls.
      :target: ../_static/snt-view/snt-view-edit.png

      Edit, check, format, and search source documents in the viewer.

Select either image to open it at full size.

Build and open
--------------

Enable the viewer and initialize its Dear ImGui and GLFW submodules before
building:

.. code-block:: sh

   git submodule update --init external/imgui external/glfw
   cmake -S . -B build -DENABLE_SNT_VIEW=ON
   cmake --build build --target snt
   build/bin/snt view examples/dip/ParameterViewer/DIPfile

The command accepts a live ``DIPfile``, a ``.dip`` or ``.dipl`` source, or an
evaluated ``.diph5`` snapshot. Viewer support requires
``ENABLE_EXEC_APPS_SNT=ON``. With ``ENABLE_SNT_VIEW=OFF``, Dear ImGui, GLFW,
and OpenGL are not compiled or linked into ``snt``. See :doc:`../dependencies`
for the general source-build requirements.

On Debian or Ubuntu, these packages provide the native files for the OpenGL,
X11, and Wayland backends:

.. code-block:: sh

   sudo apt-get install libgl-dev libglvnd-dev libx11-dev libxext-dev libxrandr-dev \
     libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libwayland-bin \
     libxkbcommon-dev wayland-protocols pkg-config

If the X11 development library is unavailable, the build skips X11. GLFW
needs ``wayland-scanner`` from ``libwayland-bin`` when its Wayland backend is
enabled.

Browse a project
----------------

The browser separates **Overridden nodes**, **Resolved nodes**, schemas, custom
units, **Local sources**, **Block value sources**, **Named sources**, and
**Raw named sources**. A live ``DIPfile`` also has a top
branch, grouping its registered inputs by type with their order preserved
within each group. Use
**Search paths** to filter the tree,
or the adjacent controls to expand and collapse its branches. Double-click a
tree node to expand or collapse it, or single-click to select it. Select a
parameter to inspect its value, units, metadata, source locations, and
dependencies. Arrays and tables expose a **Data** tab with bounded pages;
arrays of higher rank allow choosing two displayed axes and fixed indices for
the others. Use the ``<`` and ``>`` buttons beside a fixed index to step through
that dimension. Data columns initially fit the visible values up to a readable
maximum width. Drag a header divider to resize a column, scroll horizontally
to see more columns, or hover over a clipped cell to read its full value.
Values with recorded dependencies or readers expose a **Graph**
tab with a local, navigable graph and retained expression composition when
available. Follow a dependency to its input, then use the back and forward
buttons to return to earlier selections. **Local sources** lists the DIPfile's
``code[]`` inputs, which define local ``?path`` values. File inputs open their
original DIPL text; inline inputs open at their DIPfile declaration. Their
evaluated parameters appear under **Resolved nodes**.

**Block value sources** contains string literals parsed into non-string array
or table values. Array blocks are shown as plain text. Table blocks highlight
DIPL column declarations before ``---`` and show data rows as plain text.
The viewer enables this opt-in retention for live parses.
Evaluated table nodes and their columns link back to the DIPL table declaration
through **Open source**. For inline table text, **Block value sources** also
opens the parsed header and rows directly.

**Named sources** lists parsed DIPL ``sources[]`` inputs, whose values are
addressed as ``source?path``. **Raw named sources** lists unparsed text inputs
registered by ``sources[]``. A value that injects raw source content links
back to its entry when dependency recording is enabled. Raw table files use
the same header and data presentation as table block values.

The top path bar shows the argument supplied to ``snt view``. Source file
locations in the inspector are displayed relative to the opened artifact's
directory. The :ref:`ParameterViewer example <dip-parameter-viewer-example>`
provides a project with groups, collections, overrides, schemas, sources,
units, expressions, arrays, and a table for exploring these views.

Guided setup
------------

For a live ``DIPfile`` project with ``!setup`` parameters, select **Setup** in
the top menu. The menu entry appears only when setup parameters are available.
The form shows every setup parameter at once, including its
description and current value. Declared ``!options`` appear as a choice list;
Boolean values use True/False controls. Other values accept text, numbers with
units, or a DIPL array expression as appropriate. Leave a field unchanged to
keep its current value. Source files remain editable while required setup
values are still unset.

After entering the answers, click **Commit setup**. The core validates the
complete answer set before writing. A successful commit creates a new DIPL
override file beside the ``DIPfile``, registers it in the project, and reloads
the viewer so the resolved values update immediately. Invalid answers leave
the project untouched. If validation identifies an input, the form scrolls to
it and highlights it alongside the error notification. To revise a committed
answer, edit the generated override source in the viewer.
**Remove override** deletes the guided setup override file and unregisters it
from the ``DIPfile``. Other project overrides remain active. The button is
disabled until a guided setup override has been committed.

Open source and reload
----------------------

For live ``DIPfile`` projects, selecting a source opens it in the source
section; **View Source** from a node inspector navigates to the source that
defines it. Documents in the project bundle can be edited in the built-in
editor, which provides syntax highlighting, line numbers, and diagnostics with
line references. **Check** validates the current buffer using its source type:
DIPL documents use the normal DIP parser, while array and table block values
use their corresponding value parsers. **Format** formats DIPL code. Find and
replace supports previous/next navigation, replace current or all matches, and
a case-sensitivity option. **Save** writes the document and reloads the
project; **Discard** abandons unsaved edits. **Save As** copies the complete
bundled project to a chosen directory and switches the viewer to that copy.
Each file-backed override source also has a **Delete override** button. It
removes that file and its ``DIPfile`` registration after validating the
remaining project. Save or discard unsaved edits before deleting a source.
If the document changed on disk after it was opened, saving is rejected to
avoid overwriting that change. Opening another source or reloading is blocked
while the current document has unsaved edits.

Standalone ``.dip`` and ``.dipl`` inputs remain read-only in this version.
An overridden parameter offers its effective override and original
declaration. Custom units and schemas link to their definitions. The source
tab also has a **Copy source** button. For code embedded as a string in a
``DIPfile``, the action points to the string's registration line; its internal
line numbers do not refer to physical file lines.

Use **File → Reload** or **Ctrl+R** after editing an input file externally.
If the updated input fails to parse, the viewer keeps the last valid state
and displays the error. The Source tab remains open when selecting another
browser object; use **Open source** to switch to a different document.

Snapshot limits
---------------

A ``.diph5`` snapshot stores evaluated values and selected provenance, but
does not retain complete source text or the live DIPfile manifest. Source
opening and browsing named source nodes are therefore available only for live
inputs. Standalone ``.dipt`` opening and scalable access to large numerical
datasets are not yet available.

For the C++ inspection functions behind these views, see
:doc:`DIP inspection <../modules/dip/inspection>`.
