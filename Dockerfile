# Dockerfile
# =============== Stage 1: Builder ===============
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libzmq3-dev \
    python3-dev \
    python3-pip \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source & CMake
COPY CMakeLists.txt ./
COPY src/           src/
COPY include/       include/
COPY third_party/   third_party/

# Configure & build
RUN mkdir build && cd build \
 && cmake .. -DCMAKE_BUILD_TYPE=Release \
 && cmake --build . --config Release -j$(nproc) \
 && cmake --install . --prefix /app/install

# ============== Stage 2: Runtime ===============
FROM ubuntu:22.04 AS runtime

# Runtime deps only
RUN apt-get update && apt-get install -y \
    libzmq3-dev \
    python3-minimal \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binaries & installed configs from builder
COPY --from=builder /app/install/bin/      bin/
COPY --from=builder /app/install/etc/      etc/

# Expose logs & config as volume mount points
VOLUME ["/app/etc/TriangularArbitrageSystem/config", "/app/logs"]

# Set LD path & default config location
ENV LD_LIBRARY_PATH=/app/lib
ENV CONFIG_PATH=/app/etc/TriangularArbitrageSystem/config

ENTRYPOINT ["/app/bin/triangular_arbitrage"]
CMD ["--config", "/app/etc/TriangularArbitrageSystem/config/prod/execution_manager_config.json"]

