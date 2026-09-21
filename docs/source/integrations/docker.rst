Docker
======

SciNumTools provides Dockerfiles for a ready-to-use Python environment, a
reproducible development environment, and the REST API server. Build an image
from the repository root, which also supplies the source tree as the Docker
build context. The checkout must include its Git submodules.

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

REST API server
---------------

Build the image containing only the ``snt-server`` application and its native
runtime dependencies:

.. code-block:: console

   docker build \
       -f packaging/docker/rest/Dockerfile \
       -t scinumtools3-rest .

Run it as a network service:

.. code-block:: console

   docker run --rm -p 8080:8080 scinumtools3-rest

The container listens on all of its own interfaces and Docker publishes port
``8080`` on the host. The API has no authentication; production deployment
must place it behind appropriate network access controls or an
authentication-capable reverse proxy. To restrict it to the Docker host while
developing, use ``-p 127.0.0.1:8080:8080`` instead. See :doc:`REST API server
<rest>` for endpoints and request formats.
