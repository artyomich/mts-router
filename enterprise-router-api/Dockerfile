# MTS-ER-1000 Enterprise Router — Dockerfile
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake g++-12 libprotobuf-dev protobuf-compiler libgrpc++-dev \
    libboost-all-dev libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY enterprise-router-api/ .

RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=ON \
    && make -j$(nproc) \
    && make install

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libprotobuf32 libgrpc++1 \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -r mts && useradd -r -g mts -d /opt/mts -s /bin/bash mts
RUN mkdir -p /etc/mts-er1000 /var/log/mts-er1000 /var/run/mts-er1000

COPY --from=builder /usr/local/bin/mts-er1000-server /usr/local/bin/mts-er1000-server
COPY enterprise-router-api/config/mts-er1000.conf /etc/mts-er1000/config.json

RUN chown -R mts:mts /etc/mts-er1000 /var/log/mts-er1000 /var/run/mts-er1000
USER mts

EXPOSE 50054
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD ["/usr/local/bin/mts-er1000-server", "--help"]

ENTRYPOINT ["/usr/local/bin/mts-er1000-server"]
CMD ["0.0.0.0:50054"]
