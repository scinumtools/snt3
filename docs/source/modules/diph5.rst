DIPH5 — Environment HDF5 Format
================================

DIPH5 is the HDF5 interchange format used to save and load SciNumTools
environments. It maps fully qualified DIPL paths onto HDF5 groups and
datasets, and preserves collection structure, node settings, units, and
provenance through ``_DIPL_*`` attributes.

The format is independent of the Python and C++ APIs, so environments can be
exchanged with legacy scientific applications that already use HDF5.

* `Source specification <https://github.com/vrtulka23/scinumtools3/blob/main/docs/diph5/specification.md>`_

Specification
-------------

.. raw:: html

   <iframe
       src="../_static/diph5-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
   </br></br>
