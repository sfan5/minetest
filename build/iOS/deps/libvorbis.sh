#!/bin/bash -e

. ../sdk.sh
VORBIS_VERSION=1.3.5

if [ ! -d libvorbis-src ]; then
	wget http://downloads.xiph.org/releases/vorbis/libvorbis-$VORBIS_VERSION.tar.gz
	tar -xzvf libvorbis-$VORBIS_VERSION.tar.gz
	mv libvorbis-$VORBIS_VERSION libvorbis-src
	rm libvorbis-$VORBIS_VERSION.tar.gz
fi

cd libvorbis-src

CC=$IOS_CC CFLAGS=$IOS_FLAGS \
	./configure --host=arm-apple-darwin --prefix=/ \
	--disable-shared --enable-static \
	--with-ogg=$PWD/../libogg
make -j$(sysctl -n hw.ncpu)

mkdir -p ../libvorbis
make DESTDIR=$PWD/../libvorbis install

echo "libvorbis build successful"
