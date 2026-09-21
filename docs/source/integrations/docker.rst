Docker
======

SciNumTools provides Dockerfiles for a ready-to-use Python environment and a
reproducible development environment. Build either image from the repository
root, which also supplies the source tree as the Docker build context.

Python environment
------------------

Build an image for running Python code with SciNumTools installed:

.. code-block:: console

   docker build \
       -f packaging/docker/python/Dockerfile \
       -t scinumtools3-python .

Development environment
-----------------------

Build an image containing the toolchain and dependencies used to develop
SciNumTools:

.. code-block:: console

   docker build \
       -f packaging/docker/dev/Dockerfile \
       -t scinumtools3-dev .

Containers are useful for repeatable builds and for environments where
installing the complete native toolchain locally would be inconvenient.
