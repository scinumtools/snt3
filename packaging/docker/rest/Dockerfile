# -----------------------------------------------------------------------------
# SciNumTools3 REST API server
# -----------------------------------------------------------------------------

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_PARALLEL_LEVEL=4

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libhdf5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/scinumtools3
COPY . .

RUN cmake -G Ninja -S . -B build \
    -DENABLE_UNIT_TESTS=OFF \
    -DENABLE_BINDING_PYTHON=OFF \
    -DENABLE_BINDING_C=OFF \
    -DENABLE_CABI_GTEST=OFF \
    -DENABLE_EXEC_EXAMPLES=OFF \
    -DENABLE_EXEC_APPS_SNT=OFF \
    -DENABLE_EXEC_APPS_DMAP=OFF \
    -DENABLE_EXEC_APPS_SERVER=ON \
    && cmake --build build --target snt-server

EXPOSE 8080

CMD ["/opt/scinumtools3/build/bin/snt-server", "--address", "0.0.0.0", "--port", "8080"]
