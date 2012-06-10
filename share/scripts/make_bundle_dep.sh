#!/bin/bash

PACKAGE=package
PREFIX=$PACKAGE/usr
CPUS=`grep -c processor /proc/cpuinfo`
JOBS=`echo $CPUS + 1 | bc`
VERSION="0.3.0"
DEBIAN_ARCH="i386"
#DEBIAN_ARCH="amd64"

CMAKE='/usr/bin/cmake'

USER_AGENT='Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.16) Gecko/20120421 Firefox/11.0'
BOOST_URL='http://garr.dl.sourceforge.net/project/boost/boost/1.47.0/boost_1_47_0.tar.bz2'
OGRE_URL="http://garr.dl.sourceforge.net/project/ogre/ogre/1.7/ogre_src_v1-7-3.tar.bz2"

BOOST_FILENAME='boost_1_47_0.tar.bz2'
OGRE_FILENAME='ogre_src_v1-7-3.tar.bz2'
BOOST_DIR='boost_1_47_0'

BOOST_LOG_REV='607'
BOOST_GEOMETRY_EXTENSIONS_REV='77829'
MYGUI_REV='4431'

# Make sure only root can run our script
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

# Prelude
##########

function prelude
{
	# Create and enter temporary build directory
	mkdir MAKE_BUNDLE_DEP
	cd MAKE_BUNDLE_DEP

	/usr/bin/apt-get install \
		libois-dev       \
		libzip-dev       \
		libbz2-dev       \
		libcg            \
		libgl1-mesa-dev  \
		libxrandr-dev    \
		libfreeimage-dev \
		libfreetype6-dev \
		libglu-dev       \
		libxaw7-dev      \
		libzzip-dev

	/bin/mkdir -p $PREFIX


	/usr/bin/wget -U "$USER_AGENT" -c $BOOST_URL -O $BOOST_FILENAME
	/usr/bin/wget -U "$USER_AGENT" -c $OGRE_URL -O $OGRE_FILENAME

	/usr/bin/svn export -r $BOOST_LOG_REV https://boost-log.svn.sourceforge.net/svnroot/boost-log/trunk/boost-log boost-log
	/usr/bin/svn export -r $MYGUI_REV https://my-gui.svn.sourceforge.net/svnroot/my-gui/trunk my-gui
	#/usr/bin/svn export svn://connect.creativelabs.com/OpenAL/trunk OpenAL

	echo "Unpacking $BOOST_FILENAME ..."
	/bin/tar -xjf $BOOST_FILENAME

	echo "Adding Boost.Geometry arithmetic extension ..."
	svn export -r $BOOST_GEOMETRY_EXTENSIONS_REV http://svn.boost.org/svn/boost/trunk/boost/geometry/extensions $BOOST_DIR/boost/geometry/extensions

	echo "Unpacking $OGRE_FILENAME ..."
	/bin/tar -xjf $OGRE_FILENAME

	/bin/cp -r boost-log/* $BOOST_DIR

	# Bjam
	cd $BOOST_DIR
	./bootstrap.sh
	cd ..
}

function postlude
{
	# Leave temporary build directory
	cd ..
}

# Build Bjam, Boost and Boost.Log
##################################

function buildBoost
{
	cd $BOOST_DIR
	./b2                               \
		--with-date_time           \
		--with-filesystem          \
		--with-graph               \
		--with-iostreams           \
		--with-program_options     \
		--with-random              \
		--with-regex               \
		--with-serialization       \
		--with-system              \
		--with-test                \
		--with-thread              \
		define=BOOST_TEST_DYN_LINK \
		define=BOOST_TEST_MAIN     \
		variant=release            \
		threading=multi            \
		link=shared                \
		-j$JOBS                    \
		--prefix=../$PREFIX        \
		install
	cd ..
 }

function buildBoostLog
{
	cd $BOOST_DIR
	./b2                               \
		--with-log                 \
		define=BOOST_LOG_USE_CHAR  \
		variant=release            \
		threading=multi            \
		link=shared                \
		-j$JOBS                    \
		--prefix=../$PREFIX        \
		install
	cd ..
}

# Build OpenAL (requires libasound-dev)
########################################

function buildOpenAL
{
	mkdir openal-build
	cd openal-build
	$CMAKE ../OpenAL/OpenAL-Soft -DCMAKE_INSTALL_PREFIX=../$PREFIX
	make -j$JOBS install
	cd ..
}

# Build OGRE
#############

function buildOgre
{
	mkdir ogre-build
	cd ogre-build

	$CMAKE ../ogre_src_v1-7-3 \
	       -DBOOST_INCLUDEDIR=../$PREFIX/include \
	       -DBOOST_LIBRARYDIR=../$PREFIX/lib \
	       -DCMAKE_INSTALL_PREFIX=../$PREFIX

	make -j$JOBS install
	cd ..
}

# Build MyGUI
##############

function buildMyGUI
{
	# HACK: Fool MyGUI! It's too stupid to find Boost.
	ln -s ../../../../../boost_1_47_0/boost my-gui/Platforms/Ogre/OgrePlatform/include

	/bin/mkdir mygui-build
	cd mygui-build

	$CMAKE ../my-gui \
	       -DCMAKE_INSTALL_PREFIX=../$PREFIX \
	       -DOGRE_INCLUDE_DIR=../$PREFIX/include/OGRE \
	       -DOGRE_LIBRARIES=../$PREFIX/lib

	make -ik -j$JOBS install        # Skip some strange linker errors, which may hinder the installation process
	cd ..

	# install media
	/bin/mkdir -p $PREFIX/share/MyGUI
	/bin/cp -vR my-gui/Media $PREFIX/share/MyGUI
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
Package: libbfe-dev-deps
Version: $VERSION
Section: libdevel
Priority: optional
Architecture: $DEBIAN_ARCH
Depends: libsndfile-dev, libopenal-dev, libasound-dev, libois-dev, uuid-dev, libbz2-dev, libzip-dev, libbz2-dev, libfreeimage-dev, libzzip-dev
Suggests: doxygen, cmake
Conflicts: libboost-dev, libogre-dev
Installed-Size: `du -s $PREFIX | perl -pe 's#[\t]+.+##g'` 
Maintainer: Sascha Wittkowski <w177us@gmail.com>
Description: Developer package (dependencies) for the Brute Force Game Engine" > $PACKAGE/DEBIAN/control

	PACKET_FILENAME="libbfe-dev-deps_"$VERSION"_"$DEBIAN_ARCH".deb"
	/usr/bin/dpkg --build $PACKAGE $PACKET_FILENAME
	/usr/bin/md5sum $PACKET_FILENAME > $PACKET_FILENAME.md5
}

# Do it
########

prelude
buildBoost
buildBoostLog
#buildOpenAL  (unnecessary: debian package is fine)
buildOgre
buildMyGUI
makePackage
postlude

