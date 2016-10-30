#!/bin/bash -e

. ../sdk.sh
FREETYPE_VERSION=2.7

if [ ! -d freetype-src ]; then
	wget http://download.savannah.gnu.org/releases/freetype/freetype-$FREETYPE_VERSION.tar.gz
	tar -xzvf freetype-$FREETYPE_VERSION.tar.gz
	mv freetype-$FREETYPE_VERSION freetype-src
	rm freetype-$FREETYPE_VERSION.tar.gz
fi

cd freetype-src

CC=$IOS_CC CFLAGS=$IOS_FLAGS \
./configure --host=arm-apple-darwin --prefix=/ \
	--disable-shared --enable-static \
	--with-png=no
make -j2

mkdir -p ../freetype
make DESTDIR=$PWD/../freetype install

echo "freetype build successful"
