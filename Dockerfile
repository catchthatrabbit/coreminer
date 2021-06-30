# Build Ethminer in a stock Go buil
FROM gcc:latest as builder

ADD . /ethminer
RUN apt-get update
RUN apt-get install -y cmake build-essential
#RUN if [ -d "/ethminer/build" ] ; then rm -rf /ethminer/build ; fi
#RUN cd /ethminer && mkdir build && cd build && cmake .. && make -j4

# Pull Ethminer into a second stage deploy alpine container
FROM gcc:latest

RUN apt-get update
RUN apt-get install -y cmake build-essential libc6-dev

ARG address="stratum1+tcp://ab9599e9b208e60f792f87cc3fccecb143230e4f0e1b.worker@0.0.0.0:8008"
ARG cpu="--cpu"
ARG verbosity="-v 511"

COPY --from=builder /ethminer/build/ethminer/* /usr/local/bin/

ENTRYPOINT ["sh", "-c", "ethminer", "-P ${address}", "${cpu}", "${verbosity}"]
