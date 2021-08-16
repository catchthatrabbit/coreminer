# How to run:
# docker run --network host 7edb576aa48a ethminer --noeval
# -P stratum1+tcp://ab06a5eb3991c105f361e6e76840c8d5eb5eaec38021.worker@0.0.0.0:8008 --cpu -v 511
FROM gcc:latest as builder

ADD . /ethminer
RUN apt-get update
RUN apt install -y cmake build-essential
RUN if [ -d "/ethminer/build" ] ; then rm -rf /ethminer/build ; fi
RUN cd /ethminer && mkdir build && cd build && cmake .. && make -j4

# Pull Ethminer into a second stage deploy alpine container
FROM gcc:latest

RUN apt-get update
RUN apt-get install -y cmake build-essential libc6-dev

COPY --from=builder /ethminer/build/ethminer/* /usr/local/bin/
CMD ["sh", "-c", "ethminer", "--noeval"]
