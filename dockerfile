FROM gcc:14 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root

COPY . .

RUN cd build && cmake -G Ninja .. && ninja

FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libcurl4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/data

COPY --from=builder /root/build/bin/scraper /app/scraper

VOLUME ["/app/data"]

CMD ["bash"]