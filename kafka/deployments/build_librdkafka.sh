#!/bin/bash -e

cd /usr/local/src

URL=https://github.com/confluentinc/librdkafka/archive/refs/tags/v${LIBRDKAFKA_VER}.tar.gz

DIR=librdkafka-${LIBRDKAFKA_VER}

if [ ! -d ${DIR} ]; then
    wget --no-check-certificate -O - ${URL} | tar -xzf - -C ./
fi


cd ${DIR}
./configure
make
make install

cd ..
rm -r ${DIR}
