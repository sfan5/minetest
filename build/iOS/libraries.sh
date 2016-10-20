#!/bin/bash -e

cd deps

./sqlite3.sh
./irrlicht.sh

echo
echo "All libraries were built!"
