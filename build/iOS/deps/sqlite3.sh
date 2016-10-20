#!/bin/bash -e

. ../sdk.sh
SQLITE3_YEAR=2016
SQLITE3_VERSION=3150000

if [ ! -d sqlite3-src ]; then
	wget https://sqlite.org/$SQLITE3_YEAR/sqlite-amalgamation-$SQLITE3_VERSION.zip
	unzip sqlite-amalgamation-$SQLITE3_VERSION.zip
	mv sqlite-amalgamation-$SQLITE3_VERSION sqlite3-src
	rm sqlite-amalgamation-$SQLITE3_VERSION.zip
fi

cd sqlite3-src
rm -f libsqlite3.a

$IOS_CC $IOS_FLAGS sqlite3.c -O3 -c -o sqlite3.o
ar rcs libsqlite3.a sqlite3.o

mkdir -p ../sqlite3/include
cp libsqlite3.a ../sqlite3/
cp sqlite3.h ../sqlite3/include/

echo "SQLite3 build successful"
