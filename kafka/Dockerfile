FROM ubuntu:22.04

MAINTAINER duy.dinhle.hust.vn@gmail.com

ENV WD /usr/local/src/cpp_kafka
WORKDIR ${WD}

RUN apt update -y && apt upgrade -y
RUN apt install -y build-essential wget cmake libpcap-dev --no-install-recommends

# Install lz4
ENV LZ_VER=1.9.4
ENV LZ_SCRIPT=build_lz4.sh
ADD ./deployments/${LZ_SCRIPT} ${WD}/deployments/${LZ_SCRIPT}
RUN chmod +x ${WD}/deployments/${LZ_SCRIPT}
RUN ${WD}/deployments/${LZ_SCRIPT} ${LZ_VER}

# Install librdkafka
ENV LIBRDKAFKA_VER=2.3.0
ENV LIBRDKAFKA_SCRIPT=build_librdkafka.sh
ADD ./deployments/${LIBRDKAFKA_SCRIPT} ${WD}/deployments/${LIBRDKAFKA_SCRIPT}
RUN chmod +x ${WD}/deployments/${LIBRDKAFKA_SCRIPT}
RUN ${WD}/deployments/${LIBRDKAFKA_SCRIPT} ${LIBRDKAFKA_VER}

RUN ldconfig

COPY src ${WD}/src
COPY Makefile .

RUN make build-consumer

ENV INSTALLED_DIR /usr/local/cpp_kafka
WORKDIR ${INSTALLED_DIR}

RUN mkdir -p ${INSTALLED_DIR}/bin && cp ${WD}/build/consumer ${INSTALLED_DIR}/bin/consumer
RUN rm -r ${WD}

CMD ${INSTALLED_DIR}/bin/consumer
