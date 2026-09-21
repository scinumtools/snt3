REST API server
===============

The optional ``snt-server`` executable exposes the command-oriented C++ API
over HTTP. It uses the same ``snt::api`` commands as the command-line tool;
PUEL evaluation and DIPL parsing therefore retain the same semantics.

Build and run
-------------

Enable the server explicitly when configuring SNT:

.. code-block:: bash

   cmake -G Ninja -B build -DENABLE_EXEC_APPS_SERVER=ON
   cmake --build build --target snt-server
   ./build/bin/snt-server --port 8081

The server listens on ``127.0.0.1:8080`` by default. Pass ``--port PORT`` to
choose another port, for example when the default is already occupied.
``--address ADDRESS`` changes the listening address; it is intended for
container deployments, where ``0.0.0.0`` accepts connections from outside the
container. ``--help`` shows the available program options. It returns JSON and
has no authentication or authorization layer. Run it only for trusted local
users unless a reverse proxy or equivalent deployment boundary supplies those
controls.

OpenAPI document
----------------

The server exposes an OpenAPI 3.1 document at ``/openapi.json``. Load
``http://127.0.0.1:8080/openapi.json`` in Swagger UI, Swagger Editor, or any
other OpenAPI-compatible client to browse and invoke the API.

PUQ endpoints
-------------

All PUQ endpoints use ``GET`` and return ``{"result":"..."}`` on success.
Optional query parameters are named after the corresponding API command
arguments: ``input_system``, ``output_system``, ``output_units``, and
``output_quantity``.

Unit-system names select the interpretation of unit symbols; they are not
prefixes within a PUEL unit name. For example, use ``mi`` together with
``input_system=US``, not ``US_mi``.

.. code-block:: console

   $ curl --get http://127.0.0.1:8080/snt/puq/eval \
       --data-urlencode 'expression=23*cm + 3*m'
   {"result":"3.23*m"}

   $ curl --get http://127.0.0.1:8080/snt/puq/convert \
       --data-urlencode 'expression=3.048*m' \
       --data-urlencode 'output_system=US' \
       --data-urlencode 'output_units=ft'
   {"result":"10*ft"}

   $ curl --get http://127.0.0.1:8080/snt/puq/convert \
       --data-urlencode 'expression=1*mi' \
       --data-urlencode 'input_system=US' \
       --data-urlencode 'output_system=SI' \
       --data-urlencode 'output_units=km'
   {"result":"1.60934*km"}

``/snt/puq/info`` requires ``expression`` and optionally accepts
``input_system``. ``/snt/puq/list`` accepts optional ``list`` and ``system``
parameters. The list names are ``prefix``, ``base``, ``deriv``, ``log``,
``temp``, ``const``, ``quant``, and ``sys``.

DIPL endpoint
-------------

Send a DIPL document in the request body to ``POST /snt/dip/parse``. This can
be inline text or a client-side file uploaded with ``curl --data-binary``; the
server receives its contents and adds them as DIPL text. By default, the
response prints all resolved nodes. ``request`` selects a path, ``tags`` is a
comma-separated tag list, and ``value=true`` requests one unitless scalar;
``type`` may then constrain it to ``bool``, ``integer``, ``float``, or
``string``.

.. code-block:: console

   $ curl --request POST http://127.0.0.1:8080/snt/dip/parse \
       --data-binary $'length float = 2.5 m\nwidth float = 40 cm\narea float = ({?length} * {?width}) m2'
   {"result":"length = 2.5 m\nwidth = 40 cm\narea = 1 m2\n"}

   $ curl --request POST http://127.0.0.1:8080/snt/dip/parse \
       --data-binary @model.dipl
   {"result":"..."}

   $ curl --request POST 'http://127.0.0.1:8080/snt/dip/parse?request=answer&value=true&type=integer' \
       --data-binary @model.dipl
   {"result":"42\n"}

Project bundles
---------------

For an ordered multi-input environment, submit a standard :doc:`DIPfile
project <../modules/dip/projects>` and its referenced files as multipart form
uploads. The ``project`` part is the DIPfile; every ``file`` part is written
under a request-scoped temporary directory using its supplied relative
filename. The server then invokes the ordinary ``add_project`` API. Thus the
manifest retains its native ``units[]``, ``sources[]``, and ordered ``code[]``
semantics; no second REST-specific project format exists.

For example, if ``DIPfile`` refers to ``parameters/base.dip``:

.. code-block:: console

   $ curl --request POST http://127.0.0.1:8080/snt/dip/parse \
       --form 'project=@DIPfile' \
       --form 'file=@parameters/base.dip;filename=parameters/base.dip'
   {"result":"answer = 42\n..."}

Upload source files in exactly the same way, using the relative paths declared
by the project's ``sources[].filepath`` values. Absolute paths, ``.`` and
``..`` path components, duplicate paths, extra form fields, and unknown part
names are rejected. The temporary bundle directory is removed after the
request, whether parsing succeeds or fails. The endpoint therefore never
resolves a path supplied by a caller outside its isolated request bundle.

Published environments
----------------------

For deployed models, start the server with a named project or DIPH5 input:

.. code-block:: console

   snt-server --project model=/srv/model/DIPfile
   snt-server --diph5 model=/srv/model/environment.diph5

``--project NAME=PATH`` parses the ordinary DIPfile project once at startup;
``--diph5 NAME=PATH`` loads a previously evaluated environment once. Either
option may be repeated for multiple names. The inputs are then read-only and
are never selected by HTTP-supplied file paths. Restart the server to load a
changed mounted file.

``GET /snt/dip/environments`` lists published names. Retrieve all or part of
one environment with ``GET /snt/dip/environment`` using ``name`` and the same
``request``, ``tags``, ``value``, and ``type`` query parameters used by the
parse endpoint:

.. code-block:: console

   $ curl http://127.0.0.1:8080/snt/dip/environments
   {"environments":["model"]}

   $ curl --get http://127.0.0.1:8080/snt/dip/environment \
       --data-urlencode 'name=model' \
       --data-urlencode 'request=answer' \
       --data-urlencode 'value=true' \
       --data-urlencode 'type=integer'
   {"result":"42\\n"}

DIPH5 response
--------------

Add ``output=diph5`` to receive the complete evaluated environment as an
``application/x-hdf5`` attachment. The server writes it only in the
request-scoped temporary directory, returns its bytes, and then removes the
directory:

.. code-block:: console

   $ curl --request POST 'http://127.0.0.1:8080/snt/dip/parse?output=diph5' \
       --data-binary @model.dipl \
       --output environment.diph5

``output=diph5`` cannot be combined with ``value=true``. It always contains
the complete environment; request and tag filters apply only to textual
output and therefore do not filter the DIPH5 result.

The REST endpoint does not expose DIPH5 loading, direct server-side save
paths, static generation, or direct server-side file paths.

Errors
------

Invalid parameters, expressions, or DIPL input return HTTP ``400`` with a
JSON ``error`` string. For example:

.. code-block:: json

   {"error":"Missing required query parameter: expression"}
