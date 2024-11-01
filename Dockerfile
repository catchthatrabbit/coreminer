# How to run:
# docker run --network host 7edb576aa48a coreminer_runner.sh --noeval
# -P stratum1+tcp://ab06a5eb3991c105f361e6e76840c8d5eb5eaec38021.worker@0.0.0.0:8008 --cpu -v 511
FROM alpine:latest AS builder

ARG version
ADD . /coreminer
RUN apk add cmake make gcc g++ musl-dev perl linux-headers libunwind
RUN cd /coreminer && mkdir build && cd build && cmake .. -DPROJECT_VERSION=$version && make -j$(nproc)

FROM alpine:latest
RUN apk add libgcc bash
COPY --from=builder /coreminer/build/coreminer/coreminer /usr/local/bin/
COPY --from=builder /coreminer/pool.sh /usr/local/bin/
