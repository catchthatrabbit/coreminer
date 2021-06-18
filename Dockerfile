# Build Ethminer in a stock Go buil
FROM gcc:latest as builder

ADD . /ethminer
RUN apt-get update
RUN apt-get install -y cmake build-essential
RUN cd /ethminer && mkdir build && cd build && cmake .. && make -j4

# Pull Ethminer into a second stage deploy alpine container
FROM gcc:latest

COPY --from=builder /ethminer/build/ethminer/* /usr/local/bin/
