#!/bin/bash -e

. ../sdk.sh
LEVELDB_VERSION=1.19

if [ ! -d leveldb-src ]; then
	wget https://github.com/google/leveldb/archive/v$LEVELDB_VERSION.tar.gz
	tar -xzvf v$LEVELDB_VERSION.tar.gz
	mv leveldb-$LEVELDB_VERSION leveldb-src
	rm v$LEVELDB_VERSION.tar.gz
fi

cd leveldb-src

# prevent Makefile from fiddling with our flags
sed -i .old 's|^ifeq.*IOS.*|ifeq (0, 1)|g' Makefile
CC="$IOS_CC $IOS_FLAGS" CXX="$IOS_CC $IOS_FLAGS" \
TARGET_OS=IOS \
make -j$(sysctl -n hw.ncpu) out-static/libleveldb.a

[ -d ../leveldb ] && rm -r ../leveldb
mkdir -p ../leveldb/lib
cp -r include/ ../leveldb/include
cp out-static/libleveldb.a ../leveldb/lib

echo "leveldb build successful"
