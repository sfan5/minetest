#!/bin/bash -e

. ../sdk.sh

[ ! -d irrlicht-src ] && \
	svn co svn://svn.code.sf.net/p/irrlicht/code/branches/ogl-es irrlicht-src

cd irrlicht-src/

if [ ! -f .patched ]; then
	for p in touchcount unscaled dblfreefix; do
		patch -p0 <../../patches/irrlicht-$p.patch
	done
	touch .patched
fi

cd source/Irrlicht
xcodebuild build \
	-project Irrlicht.xcodeproj \
	-scheme Irrlicht_iOS \
	-destination generic/platform=iOS
cd ../..

[ -d ../irrlicht ] && rm -r ../irrlicht
mkdir -p ../irrlicht
cp lib/iOS/libIrrlicht.a ../irrlicht/
cp -r include ../irrlicht/include
cp -r media ../irrlicht/media

echo "Irrlicht build successful"
