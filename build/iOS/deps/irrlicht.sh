#!/bin/bash -e

. ../sdk.sh

[ ! -d irrlicht-src ] && \
	svn co svn://svn.code.sf.net/p/irrlicht/code/branches/ogl-es irrlicht-src


cd irrlicht-src/

cd source/Irrlicht
xcodebuild build \
	-project Irrlicht.xcodeproj \
	-scheme Irrlicht_iOS \
	-destination generic/platform=iOS
cd ../..

mkdir -p ../irrlicht
cp lib/iOS/libIrrlicht.a ../irrlicht/
cp -r include ../irrlicht/include
cp -r media ../irrlicht/media

echo "Irrlicht build successful"
