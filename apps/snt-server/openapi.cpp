#include "server.h"

namespace snt::server {

    void register_openapi_route(httplib::Server& server) {
        server.Get("/openapi.json", [](const httplib::Request&, httplib::Response& response) {
            response.set_content(
                R"json({
  "openapi": "3.1.0",
  "info": {
    "title": "SciNumTools REST API",
    "version": ")json" CODE_VERSION R"json(",
    "description": "PUEL evaluation and DIPL environment access. Mounted DIPL projects and DIPH5 environments are read-only startup inputs."
  },
  "paths": {
    "/": {
      "get": {"summary": "Service information", "responses": {"200": {"description": "Service and endpoint list"}}}
    },
    "/snt/puq/eval": {
      "get": {
        "summary": "Evaluate a PUEL expression",
        "parameters": [
          {"$ref": "#/components/parameters/expression"},
          {"$ref": "#/components/parameters/inputSystem"}
        ],
        "responses": {"200": {"$ref": "#/components/responses/result"}, "400": {"$ref": "#/components/responses/error"}}
      }
    },
    "/snt/puq/convert": {
      "get": {
        "summary": "Convert a PUEL expression",
        "parameters": [
          {"$ref": "#/components/parameters/expression"},
          {"$ref": "#/components/parameters/inputSystem"},
          {"$ref": "#/components/parameters/outputSystem"},
          {"$ref": "#/components/parameters/outputUnits"},
          {"$ref": "#/components/parameters/outputQuantity"}
        ],
        "responses": {"200": {"$ref": "#/components/responses/result"}, "400": {"$ref": "#/components/responses/error"}}
      }
    },
    "/snt/puq/info": {
      "get": {
        "summary": "Describe a PUEL expression",
        "parameters": [{"$ref": "#/components/parameters/expression"}, {"$ref": "#/components/parameters/inputSystem"}],
        "responses": {"200": {"$ref": "#/components/responses/result"}, "400": {"$ref": "#/components/responses/error"}}
      }
    },
    "/snt/puq/list": {
      "get": {
        "summary": "List PUEL definitions",
        "parameters": [
          {"name": "list", "in": "query", "schema": {"type": "string", "enum": ["prefix", "base", "deriv", "log", "temp", "const", "quant", "sys"]}},
          {"name": "system", "in": "query", "schema": {"type": "string"}}
        ],
        "responses": {"200": {"$ref": "#/components/responses/result"}, "400": {"$ref": "#/components/responses/error"}}
      }
    },
    "/snt/dip/parse": {
      "post": {
        "summary": "Parse DIPL supplied by the client",
        "description": "Send DIPL text directly, or use multipart uploads with one project part containing a DIPfile and file parts for its referenced relative files.",
        "parameters": [
          {"$ref": "#/components/parameters/request"}, {"$ref": "#/components/parameters/tags"},
          {"$ref": "#/components/parameters/value"}, {"$ref": "#/components/parameters/type"},
          {"name": "output", "in": "query", "schema": {"type": "string", "enum": ["diph5"]}}
        ],
        "requestBody": {"required": true, "content": {
          "text/plain": {"schema": {"type": "string"}},
          "application/octet-stream": {"schema": {"type": "string", "format": "binary"}},
          "multipart/form-data": {"schema": {"type": "object", "required": ["project"], "properties": {"project": {"type": "string", "format": "binary"}, "file": {"type": "array", "items": {"type": "string", "format": "binary"}}}}}
        }},
        "responses": {
          "200": {"description": "Textual result or DIPH5 attachment", "content": {"application/json": {"schema": {"$ref": "#/components/schemas/Result"}}, "application/x-hdf5": {"schema": {"type": "string", "format": "binary"}}}},
          "400": {"$ref": "#/components/responses/error"}
        }
      }
    },
    "/snt/dip/compare": {
      "post": {
        "summary": "Compare two DIPH5 snapshots",
        "description": "Compares evaluated values by default. Full scope also compares persisted metadata, provenance, settings, and manifests.",
        "parameters": [
          {"name": "scope", "in": "query", "schema": {"type": "string", "enum": ["effective", "full"], "default": "effective"}},
          {"name": "format", "in": "query", "schema": {"type": "string", "enum": ["json", "text"], "default": "json"}},
          {"name": "max_details", "in": "query", "schema": {"type": "integer", "minimum": 0, "default": 50}},
          {"name": "max_array_examples", "in": "query", "schema": {"type": "integer", "minimum": 0, "default": 3}}
        ],
        "requestBody": {"required": true, "content": {
          "multipart/form-data": {"schema": {"type": "object", "required": ["before", "after"], "properties": {
            "before": {"type": "string", "format": "binary"},
            "after": {"type": "string", "format": "binary"}
          }}}
        }},
        "responses": {
          "200": {"description": "Comparison summary", "content": {
            "application/json": {"schema": {"type": "object", "required": ["scope", "equal", "added", "removed", "changed", "differences", "omitted"], "properties": {
              "scope": {"type": "string"}, "equal": {"type": "boolean"},
              "added": {"type": "integer"}, "removed": {"type": "integer"}, "changed": {"type": "integer"},
              "differences": {"type": "array", "items": {"type": "object"}}, "omitted": {"type": "integer"}
            }}},
            "text/plain": {"schema": {"type": "string"}}
          }},
          "400": {"$ref": "#/components/responses/error"}
        }
      }
    },
    "/snt/dip/report": {
      "post": {
        "summary": "Generate a report from client-supplied DIPL",
        "description": "Accepts the same DIPL text or multipart project bundle as /snt/dip/parse. The complete evaluated environment is documented.",
        "parameters": [
          {"name": "format", "in": "query", "schema": {"type": "string", "enum": ["tex", "pdf", "md", "rst", "html", "typ", "txt", "json"], "default": "tex"}},
          {"name": "input_label", "in": "query", "schema": {"type": "string"}},
          {"name": "title", "in": "query", "schema": {"type": "string"}},
          {"name": "author", "in": "query", "schema": {"type": "string"}},
          {"name": "date", "in": "query", "schema": {"type": "string"}},
          {"name": "version", "in": "query", "schema": {"type": "string"}}
        ],
        "requestBody": {"required": true, "content": {
          "text/plain": {"schema": {"type": "string"}},
          "application/octet-stream": {"schema": {"type": "string", "format": "binary"}},
          "multipart/form-data": {"schema": {"type": "object", "required": ["project"], "properties": {"project": {"type": "string", "format": "binary"}, "file": {"type": "array", "items": {"type": "string", "format": "binary"}}}}}
        }},
        "responses": {
          "200": {"description": "Generated report attachment", "content": {"application/x-tex": {"schema": {"type": "string"}}, "application/pdf": {"schema": {"type": "string", "format": "binary"}}, "text/markdown": {"schema": {"type": "string"}}, "text/x-rst": {"schema": {"type": "string"}}, "text/html": {"schema": {"type": "string"}}, "text/plain": {"schema": {"type": "string"}}, "application/json": {"schema": {"type": "object"}}}},
          "400": {"$ref": "#/components/responses/error"}
        }
      }
    },
    "/snt/dip/environments": {
      "get": {
        "summary": "List startup-mounted environments",
        "responses": {"200": {"description": "Published environment names", "content": {"application/json": {"schema": {"type": "object", "required": ["environments"], "properties": {"environments": {"type": "array", "items": {"type": "string"}}}}}}}}
      }
    },
    "/snt/dip/environment": {
      "get": {
        "summary": "Read a startup-mounted environment",
        "description": "The environment was parsed from a mounted DIPfile or loaded from mounted DIPH5 once during server startup and is read-only.",
        "parameters": [
          {"name": "name", "in": "query", "required": true, "schema": {"type": "string"}},
          {"$ref": "#/components/parameters/request"}, {"$ref": "#/components/parameters/tags"},
          {"$ref": "#/components/parameters/value"}, {"$ref": "#/components/parameters/type"}
        ],
        "responses": {"200": {"$ref": "#/components/responses/result"}, "400": {"$ref": "#/components/responses/error"}}
      }
    }
  },
  "components": {
    "parameters": {
      "expression": {"name": "expression", "in": "query", "required": true, "schema": {"type": "string"}},
      "inputSystem": {"name": "input_system", "in": "query", "schema": {"type": "string"}},
      "outputSystem": {"name": "output_system", "in": "query", "schema": {"type": "string"}},
      "outputUnits": {"name": "output_units", "in": "query", "schema": {"type": "string"}},
      "outputQuantity": {"name": "output_quantity", "in": "query", "schema": {"type": "string"}},
      "request": {"name": "request", "in": "query", "schema": {"type": "string"}},
      "tags": {"name": "tags", "in": "query", "schema": {"type": "string"}, "description": "Comma-separated DIPL tags."},
      "value": {"name": "value", "in": "query", "schema": {"type": "boolean"}, "description": "Return one unitless scalar."},
      "type": {"name": "type", "in": "query", "schema": {"type": "string", "enum": ["bool", "integer", "float", "string"]}}
    },
    "schemas": {
      "Result": {"type": "object", "required": ["result"], "properties": {"result": {"type": "string"}}},
      "Error": {"type": "object", "required": ["error"], "properties": {"error": {"type": "string"}}}
    },
    "responses": {
      "result": {"description": "Command result", "content": {"application/json": {"schema": {"$ref": "#/components/schemas/Result"}}}},
      "error": {"description": "Invalid input", "content": {"application/json": {"schema": {"$ref": "#/components/schemas/Error"}}}}
    }
  }
})json",
                "application/vnd.oai.openapi+json;version=3.1"
            );
        });
    }

} // namespace snt::server
