#!/bin/bash
# packages needed on the rpi: libsqlite3-dev libcurl4-gnutls-dev libopenal-dev libvorbis-dev
irr=/var/tmp/irrlicht
sysroot=/var/tmp/rpi/sysroot
exec cmake . \
	-DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain.cmake \
	-DCMAKE_EXE_LINKER_FLAGS="-L$sysroot/opt/vc/lib -lbcm_host -Wl,--unresolved-symbols,ignore-in-shared-libs" \
	-DCMAKE_INSTALL_PREFIX=/tmp \
	-DENABLE_GLES=ON \
	-DRUN_IN_PLACE=ON -DENABLE_GETTEXT=OFF \
	-DIRRLICHT_LIBRARY=$irr/lib/Linux/libIrrlicht.a \
	-DIRRLICHT_INCLUDE_DIR=$irr/include
