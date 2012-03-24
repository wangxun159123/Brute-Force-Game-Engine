#!/bin/bash

PACKAGE=package
PREFIX=$PACKAGE/usr
CPUS=`grep -c processor /proc/cpuinfo`
JOBS=`echo $CPUS + 1 | bc`
VERSION="0.3.0"
ARCH="i386"

CMAKE='/usr/bin/cmake'

# Make sure only root can run our script
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

RELATIVE_BFG_ROOT_PATH="../.."

# Prelude
##########

function prelude
{
	/bin/mkdir -p $PREFIX

	cd $RELATIVE_BFG_ROOT_PATH
	./enuminit.sh
	cd -
}

# Build Bfg
############

function buildBfg
{
	mkdir bfg-build
	cd bfg-build
	$CMAKE "../$RELATIVE_BFG_ROOT_PATH" -DCMAKE_INSTALL_PREFIX="../$PREFIX"
	make -j$JOBS install
	cd ..
}

# MD5SUMS
##########

function makePackage
{
	cd $PACKAGE
	/usr/bin/find . | xargs md5sum 2&>/dev/null > ../md5sums
	/bin/mkdir -p DEBIAN
	/bin/mv ../md5sums DEBIAN
	cd ..

	/usr/bin/find $PREFIX -type d | xargs /bin/chmod 755

	echo "
Package: libbfe-dev
Version: $VERSION
Section: libdevel
Priority: optional
Architecture: $ARCH
Depends: libbfe-dev-deps
Suggests: 
Conflicts: 
Installed-Size: `du -s $PREFIX | perl -pe 's#[\t]+.+##g'` 
Maintainer: Sascha Wittkowski <w177us@gmail.com>
Description: The Brute Force Game Engine
	" > $PACKAGE/DEBIAN/control

	PACKET_FILENAME="libbfe-dev_"$VERSION"_"$ARCH".deb"
	/usr/bin/dpkg --build $PACKAGE $PACKET_FILENAME
	/usr/bin/md5sum $PACKET_FILENAME > $PACKET_FILENAME.md5
}

# Do it
########

prelude
buildBfg
makePackage

