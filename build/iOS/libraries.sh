#!/bin/bash -e

cd deps

./irrlicht.sh
./libogg.sh
./libvorbis.sh # depends on libogg
./leveldb.sh
./freetype.sh

echo
echo "All libraries were built!"
