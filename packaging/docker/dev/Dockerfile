# -----------------------------------------------------------------------------
# SciNumTools3 Development Environment
# -----------------------------------------------------------------------------

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# System packages
# -----------------------------------------------------------------------------

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    gdb \
    lldb \
    clang \
    clang-format \
    clang-tidy \
    ccache \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# Python development tools
# -----------------------------------------------------------------------------

RUN python3 -m pip install --break-system-packages \
    numpy \
    scikit-build-core \
    pybind11 \
    pytest \
    pytest-cmake \
    pytest-benchmark \
    sphinx \
    sphinx-rtd-theme \
    breathe \
    black \
    ruff \
    mypy

# -----------------------------------------------------------------------------
# Build defaults
# -----------------------------------------------------------------------------

ENV CC=clang
ENV CXX=clang++
ENV CMAKE_GENERATOR=Ninja
ENV CMAKE_BUILD_PARALLEL_LEVEL=4

# -----------------------------------------------------------------------------
# Workspace
# -----------------------------------------------------------------------------

WORKDIR /workspace

CMD ["bash"]