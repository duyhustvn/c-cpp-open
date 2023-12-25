#!/bin/bash -e

cd /usr/local/src

URL=https://github.com/lz4/lz4/releases/download/v${LZ_VER}/lz4-${LZ_VER}.tar.gz

DIR=lz4-${LZ_VER}

if [ ! -d ${DIR} ]; then
    # mkdir -p ./${DIR}
    # wget --no-check-certificate -O - ${URL} | tar -xfz - -C ./${DIR}
    wget --no-check-certificate -O - ${URL} | tar -xzf - -C ./
fi

cd ${DIR}
make
make install

cd ..
rm -r ${DIR}
