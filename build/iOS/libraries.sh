#!/bin/bash -e

cd deps

./irrlicht.sh
./libogg.sh
./libvorbis.sh # depends on libogg

echo
echo "All libraries were built!"
