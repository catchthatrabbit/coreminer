# How to run:
# docker run --network host 7edb576aa48a ethminer --noeval
# -P stratum1+tcp://ab06a5eb3991c105f361e6e76840c8d5eb5eaec38021.worker@0.0.0.0:8008 --cpu -v 511
FROM alpine:3 as builder

ADD . /ethminer
RUN apk add cmake make gcc g++  musl-dev perl linux-headers libexecinfo-dev libunwind
RUN cd /ethminer && mkdir build && cd build && cmake .. && make -j4

FROM alpine:3
RUN apk add libgcc
COPY --from=builder /ethminer/build/ethminer/* /usr/local/bin/
CMD ["sh", "-c", "ethminer_runner.sh"]
