# MTS-RG-500 Residential Gateway — Dockerfile
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake make g++ libprotobuf-dev protobuf-compiler libgrpc++-dev \
    libboost-all-dev libgtest-dev libgrpc-dev protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY residential-gateway-api/ .

RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON \
    -DgRPC_DIR=/usr/lib/x86_64-linux-gnu/cmake/grpc \
    && make -j$(nproc) \
    && make install

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libprotobuf32t64 \
    libgrpc++1.51t64 \
    libboost-system1.83.0 \
    libboost-filesystem1.83.0 \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -r mts && useradd -r -g mts -d /opt/mts -s /bin/bash mts
RUN mkdir -p /etc/mts-rg500 /var/log/mts-rg500 /var/run/mts-rg500

COPY --from=builder /usr/local/bin/mts-rg500-server /usr/local/bin/mts-rg500-server
COPY residential-gateway-api/config/mts-rg500.conf /etc/mts-rg500/config.json

RUN chown -R mts:mts /etc/mts-rg500 /var/log/mts-rg500 /var/run/mts-rg500
USER mts

EXPOSE 50055
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD ["/usr/local/bin/mts-rg500-server", "--help"]

ENTRYPOINT ["/usr/local/bin/mts-rg500-server"]
CMD ["0.0.0.0:50055"]
