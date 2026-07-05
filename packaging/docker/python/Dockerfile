# -----------------------------------------------------------------------------
# SciNumTools3 Docker image
# Ubuntu 24.04 + C++17 + Python
# -----------------------------------------------------------------------------

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_PARALLEL_LEVEL=4


# System packages
# -----------------------------------------------------------------------------

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# Python build dependencies
# -----------------------------------------------------------------------------

RUN python3 -m pip install --break-system-packages \
    setuptools \
    wheel \
    numpy \
    pybind11 \
    scikit-build-core \
    pytest \
    pytest-cmake \
    pytest-benchmark

# -----------------------------------------------------------------------------
# Copy source tree
# -----------------------------------------------------------------------------

WORKDIR /opt/scinumtools3
COPY . .

# -----------------------------------------------------------------------------
# Build and install SciNumTools3
# -----------------------------------------------------------------------------

RUN python3 -m pip install --break-system-packages . \
    --config-settings=cmake.define.ENABLE_MAT=OFF \
    --config-settings=cmake.define.ENABLE_UNIT_TESTS=OFF \
    --config-settings=cmake.define.RUN_UNIT_TESTS=OFF \
    --config-settings=cmake.define.ENABLE_EXEC_APPS_DMAP=OFF \
    --config-settings=cmake.define.ENABLE_EXEC_BENCHMARKS=OFF

# -----------------------------------------------------------------------------
# Verify installation
# -----------------------------------------------------------------------------

RUN python3 - <<EOF
import scinumtools3
print("SciNumTools3 version:", scinumtools3.__version__)
EOF

# -----------------------------------------------------------------------------
# Default working directory
# -----------------------------------------------------------------------------

WORKDIR /workspace

CMD ["bash"]