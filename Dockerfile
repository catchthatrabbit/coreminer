# Build Ethminer in a stock Go buil
FROM gcc:latest as builder

ADD . /ethminer
RUN apt-get update
RUN apt-get install -y cmake build-essential
RUN cd /ethminer && mkdir build && cd build && cmake .. && make -j4

# Pull Ethminer into a second stage deploy alpine container
FROM gcc:latest

RUN apt-get update
RUN apt-get install -y cmake build-essential libc6-dev

ENV address stratum1+tcp://ab9599e9b208e60f792f87cc3fccecb143230e4f0e1b.worker@0.0.0.0:8008
ENV cpu FALSE
ENV verbosity FALSE

ENV params="-P ${address}"

RUN if [ $cpu = TRUE ] ; then $params="${params} --cpu" ; else $params=$params ; fi

RUN if [ $verbosity = TRUE ] ; then $params="${params} -v 511" ; else params=$params ; fi

COPY --from=builder /ethminer/build/ethminer/* /usr/local/bin/

ENTRYPOINT ["sh", "-c", "ethminer ${params}"]
