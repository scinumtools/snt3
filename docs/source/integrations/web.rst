WebAssembly and JavaScript
==========================

The experimental DIPL web integration runs the existing C++ parser and
inspector in a browser through WebAssembly. Its JavaScript module provides
string-based parsing, value descriptions, schema inspection, validation, and
override reevaluation. It has no browser UI dependency. The interface is
currently a local prototype and is not a published npm package.

Build it with Emscripten's ``emcmake`` and a C++17-capable toolchain:

.. code-block:: shell

   export EM_CACHE="$PWD/build-web/cache"
   emcmake cmake -S bindings/web -B build-web -G Ninja
   cmake --build build-web -j4
   node bindings/web/tests/smoke.mjs

Emscripten also requires Python 3.10 or newer. Put that ``python3`` first on
``PATH`` if your operating system provides an older default. ``EM_CACHE``
keeps the compiler cache in a writable project build directory.

The build writes ``bindings/web/dist/dipl_web.mjs`` and
``bindings/web/dist/dipl_web.wasm``. When serving these files to a browser,
serve the JavaScript and WASM from the same origin and use a server that sends
``application/wasm`` for the ``.wasm`` file.

The module initializes asynchronously. Evaluated values are returned as DIPL
text, including units separately, so callers do not lose integer precision or
array type information through an implicit JavaScript conversion:

.. code-block:: javascript

   import { loadDIPL } from "./bindings/web/index.mjs";

   const dipl = await loadDIPL();
   const source = `distance float = 24 m
   duration float = 8 s
   speed float = ({?distance} / {?duration}) m/s`;
   const model = dipl.parse(source);
   console.log(model.paths());
   console.log(model.describe("speed").valueText);

   const candidate = model.withOverride("distance = 32 m");
   console.log(candidate.describe("speed").valueText);
   console.log(dipl.validate(source, "missing = 1"));
   candidate.dispose();
   model.dispose();

``withOverride`` evaluates the original source with a replacement override
body and returns a new model. It does not mutate the original. This uses the
same parser and validation rules as native DIPL. ``schemas()`` exposes the
schema definition and application hierarchy for building navigation or forms.
See :doc:`../modules/dip/inspection` for the related C++ inspection API.

For a model split across several sources, pass text-backed registrations to
``parseProject``. This calls the core C++ project registration method, which
processes units, named schema bodies, existing overrides, and ordered code in
the same phases as a DIPfile. Code and override ``name`` fields are source
identities used in diagnostics; schema and unit ``name`` fields are their DIPL
names. Schema bodies and override bodies have no ``$schema`` or ``$override``
wrapper. A website can fetch or generate this object without providing a
filesystem to the parser:

.. code-block:: javascript

   const project = {
     units: [{ name: "custom_length", text: "m" }],
     schemas: [{ name: "settings", text: "distance float = 24 custom_length\nduration float = 8 s" }],
     code: [
       { name: "model.dip", text: "simulation : settings" },
       { name: "derived.dip", text: "speed float = ({?simulation.distance} / {?simulation.duration}) m/s" },
     ],
     overrides: [],
   };
   const model = dipl.parseProject(project);
   const next = model.withOverride("simulation.distance = 32 custom_length");
   console.log(next.describe("speed").valueText);
   console.log(dipl.validateProject(project, "unknown = 1"));
   next.dispose();
   model.dispose();

``withOverride`` replaces the previous interactive override body and leaves
``project.overrides`` intact. DIPL rejects two overrides of the same path. If the
project input already contains an override for the path being edited, replace
that entry in ``project.overrides`` and call ``parseProject`` again; the web
binding retains the core parser's duplicate-override rule.

The project input is an in-memory registration list, not a DIPfile parser. A
site preparing assets from a DIPfile must resolve its referenced files and
preserve their code order when it builds this object. Filesystem-backed source
imports, DIPH5 persistence, reports, adapters, and DIPL source serialization
are outside this prototype. For an editable configuration, retain the original
project input and the accepted override body; the core does not currently
provide a DIPL round-trip serializer.
