# Docker Images

This directory contains Docker images for **SciNumTools3**.

## Available Images

| Image     | Purpose                                                                    |
| --------- | -------------------------------------------------------------------------- |
| `python/` | Ready-to-use Python environment with SciNumTools3 installed.               |
| `dev/`    | Development environment for building, testing, and modifying SciNumTools3. |
| `rest/`   | REST API server exposing SciNumTools3 functionality.                       |
| `mcp/`    | MCP over HTTP or stdio, reusing the REST image's SNT executable.           |

---

# Building Images

All images should be built from the **repository root**, specifying the desired Dockerfile with the `-f` option.

## Python Image

Build:

```bash
docker build \
    -f packaging/docker/python/Dockerfile \
    -t scinumtools3-python .
```

Run:

```bash
docker run --rm -it scinumtools3-python
```

Verify the installation:

```bash
python3 -c "import scinumtools3; print(scinumtools3.__version__)"
```

---

## Development Image

Build:

```bash
docker build \
    -f packaging/docker/dev/Dockerfile \
    -t scinumtools3-dev .
```

Run the container with the repository mounted into the workspace:

```bash
docker run --rm -it \
    -v "$(pwd)":/workspace/scinumtools3 \
    scinumtools3-dev
```

Inside the container:

```bash
cd /workspace/scinumtools3
```

Configure the project:

```bash
cmake -S . -B build
```

> [!NOTE] Don't forget to remove old 'build' directory if one already exists!

Build:

```bash
cmake --build build
```

Run the C++ test suite:

```bash
ctest --test-dir build --output-on-failure
```

Install the Python package into the container:

```bash
python3 -m pip install --break-system-packages -e .
```

or

```bash
python3 -m pip install --break-system-packages .
```

Run the Python tests:

```bash
pytest
```

---

## REST API Server Image

Build:

```bash
docker build \
    -f packaging/docker/rest/Dockerfile \
    -t scinumtools3-rest .
```

Generate a token and run the HTTP listener on a host-loopback published port:

```bash
umask 077; openssl rand -hex 32 > snt-http.token
docker run --rm -p 127.0.0.1:8080:8080 \
    -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
    scinumtools3-rest --auth-token-file /run/snt-http.token
```

The container binds to all its own interfaces, so its entry point requires a
token file. Send `Authorization: Bearer TOKEN` with each request. Keep the
published port restricted to host loopback and forward remote traffic through
an HTTPS reverse proxy on that host; a token over plain public HTTP is unsafe.
The token file must have mode `0600` on Unix. See the
[REST integration guide](../../docs/source/integrations/rest.rst) for a Caddy
example and the single-token access limits.

Publish a mounted DIPL project without uploading its inputs over HTTP:

```bash
docker run --rm -p 127.0.0.1:9000:8080 \
    -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
    -v "$(pwd)/model:/data:ro" \
    scinumtools3-rest \
    --auth-token-file /run/snt-http.token --project model=/data/DIPfile
```

Use ``--diph5 model=/data/environment.diph5`` with a read-only mounted DIPH5
file to publish an already evaluated environment.

---

# Notes

For MCP clients, build the REST image above, then:

```bash
docker build -f packaging/docker/mcp/Dockerfile -t scinumtools3-mcp .
docker run --rm -p 127.0.0.1:8081:8081 \
    --user "$(id -u):$(id -g)" \
    -v "$PWD/snt-http.token:/run/snt-http.token:ro" \
    -v "$PWD/model:/data:ro" scinumtools3-mcp \
    --transport streamable-http --address 0.0.0.0 \
    --auth-token-file /run/snt-http.token --project model=/data/DIPfile
```

The MCP endpoint is `http://localhost:8081/mcp` and requires the same bearer
token. Docker arguments replace the
default `CMD`, so include the transport and bind address when adding projects.
See [the MCP adapter guide](../../apps/snt-mcp/README.md) for stdio configuration,
available tools, and access controls. The REST image remains a separate service.

* Always execute `docker build` from the repository root.
* The development image does **not** contain the SciNumTools3 sources. Instead, the repository is mounted into the container, allowing changes made on the host to be immediately visible inside the container without rebuilding the image.
* The Python image is intended for users who simply want a ready-to-use SciNumTools3 installation.

The REST image builds the `snt` target with `ENABLE_SNT_SERVER=ON` and
`ENABLE_SNT_VIEW=OFF`. Its entry point is `snt server --address 0.0.0.0`;
container arguments remain server options such as `--auth-token-file`, `--port`,
or `--project`.
