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

Build the image containing ``snt`` with server support and its native
runtime dependencies:

.. code-block:: console

   docker build \
       -f packaging/docker/rest/Dockerfile \
       -t scinumtools3-rest .

Generate a token and publish the service on the Docker host's loopback:

.. code-block:: console

   umask 077; openssl rand -hex 32 > snt-http.token
   docker run --rm -p 127.0.0.1:8080:8080 \
       -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
       scinumtools3-rest --auth-token-file /run/snt-http.token

The container listens on all of its own interfaces, so it requires a token
file. Its host port is loopback-only. Send ``Authorization: Bearer TOKEN`` with
every request; for remote use, forward through an HTTPS reverse proxy without
exposing the backend port. See :doc:`REST API server <rest>` for an example and
for the single-token access limits.

Published parameter environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Mount a DIPL project directory read-only and publish the environment parsed
from its ``DIPfile`` at startup:

.. code-block:: console

   docker run --rm -p 127.0.0.1:9000:8080 \
       -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
       -v "$(pwd)/model:/data:ro" \
       scinumtools3-rest \
       --auth-token-file /run/snt-http.token --project model=/data/DIPfile

The service is then available on port ``9000``. It never receives the project
files over HTTP; it parses the mounted project once and exposes the immutable
result under the name ``model``. A mounted DIPH5 environment works similarly:

.. code-block:: console

   docker run --rm -p 127.0.0.1:9000:8080 \
       -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
       -v "$(pwd)/environment.diph5:/data/environment.diph5:ro" \
       scinumtools3-rest \
       --auth-token-file /run/snt-http.token \
       --diph5 model=/data/environment.diph5
