# Docker Images

This directory contains Docker images for **SciNumTools3**.

## Available Images

| Image     | Purpose                                                                    |
| --------- | -------------------------------------------------------------------------- |
| `python/` | Ready-to-use Python environment with SciNumTools3 installed.               |
| `dev/`    | Development environment for building, testing, and modifying SciNumTools3. |
| `rest/`   | *(Planned)* REST API server exposing SciNumTools3 functionality.           |

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

# Notes

* Always execute `docker build` from the repository root.
* The development image does **not** contain the SciNumTools3 sources. Instead, the repository is mounted into the container, allowing changes made on the host to be immediately visible inside the container without rebuilding the image.
* The Python image is intended for users who simply want a ready-to-use SciNumTools3 installation.
