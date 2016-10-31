#!/bin/bash -e

. ../sdk.sh
OGG_VERSION=1.3.2

if [ ! -d libogg-src ]; then
	wget http://downloads.xiph.org/releases/ogg/libogg-$OGG_VERSION.tar.gz
	tar -xzvf libogg-$OGG_VERSION.tar.gz
	mv libogg-$OGG_VERSION libogg-src
	rm libogg-$OGG_VERSION.tar.gz
fi

cd libogg-src

CC=$IOS_CC CFLAGS=$IOS_FLAGS \
./configure --host=arm-apple-darwin --prefix=/ \
	--disable-shared --enable-static
make -j$(sysctl -n hw.ncpu)

mkdir -p ../libogg
make DESTDIR=$PWD/../libogg install

echo "libogg build successful"
