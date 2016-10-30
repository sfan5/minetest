#!/bin/bash -e

if [ ! -d Minetest/Minetest.xcodeproj ]; then
	echo "Run this in build/iOS"
	exit 1
fi

DEST=$(mktemp -d)

for dir in builtin client textures; do
	cp -r ../../$dir $DEST/$dir
done
mkdir -p $DEST/fonts
cp ../../fonts/*.ttf $DEST/fonts/ # PNG fonts useless with freetype
mkdir -p $DEST/games
if [ -d ../../games/minetest_game ]; then # copy only one subgame
	cp -r ../../games/minetest_game $DEST/games/minetest_game
else
	cp -r ../../games/minimal $DEST/games/minimal
fi
mkdir -p $DEST/media
cp -r deps/irrlicht/media/Shaders $DEST/media/Shaders

find $DEST -type d -name '.git' -print0 | xargs -0 -- rm -r
find $DEST -type f -name '.git*' -delete
find $DEST -type f -name '.DS_Store' -delete

echo "Creating assets.zip"
ZIPDEST=$(pwd)/assets.zip
rm -f $ZIPDEST

cd $DEST; zip -9r $ZIPDEST -- *
cd /; rm -rf $DEST
