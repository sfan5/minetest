#!/bin/bash -e

# Generates different AppIcon images with correct dimensions
# (brew package: librsvg)
SIZES="76 120 152 167 180"
SRCFILE=../../misc/minetest.svg
DSTDIR=Minetest/Minetest/Assets.xcassets/AppIcon.appiconset

for sz in $SIZES; do
	echo "Creating ${sz}x${sz} icon"
	rsvg-convert -b white -w ${sz} -h ${sz} -o $DSTDIR/AppIcon-${sz}.png $SRCFILE
done
