# MTS-CR-9000 Core Router — Dockerfile
# Multi-stage build for minimal production image

# ============================================================
# Stage 1: Builder
# ============================================================
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake g++-12 libprotobuf-dev protobuf-compiler libgrpc++-dev \
    libboost-all-dev libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY core-router-api/ .

RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON \
    && make -j$(nproc) \
    && make install

# ============================================================
# Stage 2: Runtime
# ============================================================
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libprotobuf32 \
    libgrpc++1 \
    libboost-system1.74.0 \
    libboost-filesystem1.74.0 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN groupadd -r mts && useradd -r -g mts -d /opt/mts -s /bin/bash mts

# Create directories
RUN mkdir -p /etc/mts-cr9000 \
    /var/log/mts-cr9000 \
    /var/run/mts-cr9000

# Copy binary
COPY --from=builder /usr/local/bin/mts-cr9000-server /usr/local/bin/mts-cr9000-server

# Copy default config
COPY core-router-api/config/mts-cr9000.conf /etc/mts-cr9000/config.json

# Set ownership
RUN chown -R mts:mts /etc/mts-cr9000 /var/log/mts-cr9000 /var/run/mts-cr9000

USER mts

# Expose gRPC port
EXPOSE 50051

# Health check
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD ["/usr/local/bin/mts-cr9000-server", "--help"]

ENTRYPOINT ["/usr/local/bin/mts-cr9000-server"]
CMD ["0.0.0.0:50051"]
