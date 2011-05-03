
# start here
cd osx/sources

# build icu and boost for packaging up within Mapnik Framework

# local install location
PREFIX=/Users/dane/projects/mapnik-dev/trunk-build/osx/sources
mkdir -p /Users/dane/projects/mapnik-dev/trunk-build/osx/sources
export DYLD_LIBRARY_PATH=$PREFIX/lib
# final resting place
INSTALL=/Library/Frameworks/Mapnik.framework/unix/lib
export DYLD_LIBRARY_PATH=$PREFIX/lib
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
# To ensure things like pkg-config built locally are used
export PATH=$PREFIX/bin:$PATH


# make a directory to hold icu and boost
mkdir -p ../deps
cd ../deps

# ICU
wget http://download.icu-project.org/files/icu4c/4.6/icu4c-4_6-src.tgz
tar xvf icu4c-4_6-src.tgz
cd icu/source

# universal flags
export CFLAGS="-O3 -arch i386 -arch x86_64"
export CXXFLAGS="-O3 -arch i386 -arch x86_64"
export LDFLAGS="-arch i386 -arch x86_64 -headerpad_max_install_names"
#./runConfigureICU MacOSX --prefix=$PREFIX --disable-static --enable-shared --disable-samples --disable-icuio --disable-layout --disable-tests --disable-extras --with-library-bits=64
./runConfigureICU MacOSX --prefix=$PREFIX --disable-static --enable-shared --with-library-bits=64
make install -j4
# note -R is needed to preserve the symlinks
#cp -R lib/libicuuc.* ../../../sources/lib/
#cp -R lib/libicudata.* ../../../sources/lib/
cd ../../../sources/lib/


# libicuuc
install_name_tool -id $INSTALL/libicuuc.46.dylib libicuuc.46.0.dylib
# --enable-layout
#install_name_tool -change ../lib/libicudata.46.0.dylib $INSTALL/libicudata.46.dylib libicuuc.46.0.dylib
#--disable-layout
install_name_tool -change libicudata.46.dylib $INSTALL/libicudata.46.dylib libicuuc.46.0.dylib
# libicudata
install_name_tool -id $INSTALL/libicudata.46.dylib libicudata.46.0.dylib
# libicui18n - needed by boost_regex
install_name_tool -id $INSTALL/libicui18n.46.dylib libicui18n.46.0.dylib
# --enable-layout
#install_name_tool -change ../lib/libicudata.46.0.dylib $INSTALL/libicudata.46.dylib libicui18n.46.0.dylib
#--disable-layout
install_name_tool -change libicudata.46.dylib $INSTALL/libicudata.46.dylib libicui18n.46.0.dylib
install_name_tool -change libicuuc.46.dylib $INSTALL/libicuuc.46.dylib libicui18n.46.0.dylib
cd ../


# freetype2
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.4.tar.gz
tar xvf freetype-2.4.4.tar.gz
export CFLAGS="-O3 -arch i386 -arch x86_64"
export LDFLAGS="-arch i386 -arch x86_64 -headerpad_max_install_names"
cd freetype-2.4.4
./configure --prefix=$PREFIX
make -j4
make install
install_name_tool -id $INSTALL/libfreetype.6.dylib ../../sources/lib/libfreetype.6.dylib
cd ../

# pkg-config so we get cairo and friends configured correctly
wget http://pkgconfig.freedesktop.org/releases/pkg-config-0.25.tar.gz
tar xvf pkg-config-0.25.tar.gz
cd pkg-config-0.25
./configure --disable-dependency-tracking --prefix=$PREFIX

# pixman
wget http://cairographics.org/releases/pixman-0.20.2.tar.gz
tar xvf pixman-0.20.2.tar.gz
cd pixman-0.20.2
./configure --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../../sources/lib/
install_name_tool -id $INSTALL/libpixman-1.0.dylib ../../sources/lib/libpixman-1.0.dylib
cd ../

# fontconfig
wget http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.8.0.tar.gz
tar xvf fontconfig-2.8.0.tar.gz
cd fontconfig-2.8.0
./configure --disable-dependency-tracking --prefix=$PREFIX \
    --with-freetype-config=$PREFIX/bin/freetype-config
make -j4
make install
install_name_tool -id $INSTALL/libfontconfig.1.dylib ../../sources/lib/libfontconfig.1.dylib
cd ../

# Cairo
wget http://cairographics.org/releases/cairo-1.10.2.tar.gz
tar xvf cairo-1.10.2.tar.gz
cd cairo-1.10.2
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export LDFLAGS="-L/Library/Frameworks/UnixImageIO.framework/unix/lib "$LDFLAGS
export CFLAGS="-I/Library/Frameworks/UnixImageIO.framework/unix/include "$CFLAGS
export png_CFLAGS="-I/Library/Frameworks/UnixImageIO.framework/unix/include"
export png_LIBS="-I/Library/Frameworks/UnixImageIO.framework/unix/lib -lpng14"
./configure \
  --disable-valgrind \
  --enable-gobject=no \
  --enable-static=no \
  --enable-xlib=no \
  --enable-xlib-xrender=no \
  --enable-xcb=no \
  --enable-xlib-xcb=no \
  --enable-xcb-shm=no \
  --enable-xcb-drm=no \
  --disable-dependency-tracking \
  --prefix=$PREFIX
  
make -j4
make install
install_name_tool -id $INSTALL/libcairo.2.dylib ../../sources/lib/libcairo.2.dylib
cd ../

# since linking to libpng framework (which does not provide a pkg-config) fake it:

prefix=/Library/Frameworks/UnixImageIO.framework/unix
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: cairo-png
Description: PNG functions for cairo graphics library
Version: 1.10.2

Requires: cairo libpng
Libs:  
Cflags: -I${includedir}/cairo 


# libsigcxx
wget http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.2/libsigc++-2.2.8.tar.gz
tar xvf libsigc++-2.2.8.tar.gz
cd libsigc++-2.2.8
./configure --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
install_name_tool -id $INSTALL/libsigc-2.0.dylib ../../sources/lib/libsigc-2.0.dylib
cd ../

wget http://cairographics.org/releases/cairomm-1.9.8.tar.gz
tar xvf cairomm-1.9.8.tar.gz
cd cairomm-1.9.8
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export LDFLAGS="-L$PREFIX/lib -lcairo -lsigc-2.0 "$LDFLAGS
export CFLAGS="-I$PREFIX/include -I$PREFIX/include/cairo -I$PREFIX/include/freetype2 -I$PREFIX/lib/sigc++-2.0/include -I$PREFIX/include/sigc++-2.0 -I$PREFIX/include/sigc++-2.0/sigc++ "$CFLAGS
export CXXFLAGS="-I$PREFIX/include "$CFLAGS

./configure --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install

install_name_tool -id $INSTALL/libcairomm-1.0.1.dylib ../../sources/lib/libcairomm-1.0.1.dylib

# also make sure cairo and friends did not link against anything in /opt/local or /usr/local
otool -L ../../sources/lib/*dylib | grep local

# pycairo
# >= python 3.1
#wget http://cairographics.org/releases/pycairo-1.8.10.tar.bz2
#tar xvf pycairo-1.8.10.tar.bz2
#./waf configure

#wget http://cairographics.org/releases/pycairo-1.8.8.tar.gz
#tar xvf pycairo-1.8.8.tar.gz
#cd pycairo-1.8.8

# py25
# line 35 of configure.ac AM_PATH_PYTHON(2.5)
#export PATH=/Library/Frameworks/Python.framework/Versions/2.5/bin/:$PATH
#./configure --prefix=$PREFIX
#make -j4 install

# py26
#export PATH=/Library/Frameworks/Python.framework/Versions/2.6/bin/:$PATH
#./configure --prefix=$PREFIX
#make -j4 install

#py27
#export PATH=/Library/Frameworks/Python.framework/Versions/2.7/bin/:$PATH
#make clean
#./configure --prefix=$PREFIX
#make -j4 install


# boost
wget http://voxel.dl.sourceforge.net/project/boost/boost/1.46.1/boost_1_46_1.tar.bz2
tar xjvf boost_1_46_1.tar.bz2
cd boost_1_46_1

# edit tools/build/v2/tools/python.jam, line 980, replace with:
    if $(target-os) in windows cygwin
    {
        alias python_for_extensions : python : $(target-requirements) ;
    }
    else if $(target-os) = darwin
    {
        alias python_for_extensions
            :
            : $(target-requirements)
            :
            : $(usage-requirements) <linkflags>"-undefined dynamic_lookup"
            ;
    }

./bootstrap.sh
./bjam --prefix=$PREFIX --with-python --with-thread --with-filesystem \
  --with-regex --with-program_options --with-system \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=32_64 \
  architecture=x86 \
  link=shared \
  variant=release \
  stage

./bjam --prefix=$PREFIX --with-python --with-thread --with-filesystem \
  --with-regex --with-program_options --with-system \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=32_64 \
  architecture=x86 \
  link=shared \
  variant=release \
  install

# boost python for various versions are done in python script
python ../../scripts/build_boost_pythons.py 2.5 32_64
mv stage/lib/libboost_python.dylib stage/lib/libboost_python25.dylib
cp stage/lib/libboost_python25.dylib ../../sources/lib/libboost_python25.dylib

python ../../scripts/build_boost_pythons.py 2.6 32_64
mv stage/lib/libboost_python.dylib stage/lib/libboost_python26.dylib
cp stage/lib/libboost_python26.dylib ../../sources/lib/libboost_python26.dylib

python ../../scripts/build_boost_pythons.py 2.7 32_64
mv stage/lib/libboost_python.dylib stage/lib/libboost_python27.dylib
cp stage/lib/libboost_python27.dylib ../../sources/lib/libboost_python27.dylib

python ../../scripts/build_boost_pythons.py 3.1 32_64
mv stage/lib/libboost_python3.dylib stage/lib/libboost_python31.dylib
cp stage/lib/libboost_python31.dylib ../../sources/lib/libboost_python31.dylib

python ../../scripts/build_boost_pythons.py 3.2 32_64
mv stage/lib/libboost_python3.dylib stage/lib/libboost_python32.dylib
cp stage/lib/libboost_python32.dylib ../../sources/lib/libboost_python32.dylib


#cp stage/lib/libboost_*dylib ../../sources/lib/

cd ../../sources/lib

# fix boost pythons
install_name_tool -id $INSTALL/libboost_python25.dylib libboost_python25.dylib
install_name_tool -id $INSTALL/libboost_python26.dylib libboost_python26.dylib
install_name_tool -id $INSTALL/libboost_python27.dylib libboost_python27.dylib
install_name_tool -id $INSTALL/libboost_python31.dylib libboost_python31.dylib
install_name_tool -id $INSTALL/libboost_python32.dylib libboost_python32.dylib

# fix boost libs
install_name_tool -id $INSTALL/libboost_system.dylib libboost_system.dylib
install_name_tool -id $INSTALL/libboost_filesystem.dylib libboost_filesystem.dylib
install_name_tool -id $INSTALL/libboost_regex.dylib libboost_regex.dylib
install_name_tool -id $INSTALL/libboost_program_options.dylib libboost_program_options.dylib
install_name_tool -id $INSTALL/libboost_thread.dylib libboost_thread.dylib
install_name_tool -change libboost_system.dylib $INSTALL/libboost_system.dylib libboost_filesystem.dylib
#install_name_tool -change libicui18n.46.dylib $INSTALL/libicui18n.46.dylib libboost_regex.dylib

# currently broken, waiting on http://www.gaia-gis.it/rasterlite2/
# rasterlite we must bundle as it is not available in the SQLite.framework
#cd ../../deps
#svn co https://www.gaia-gis.it/svn/librasterlite
#export LDFLAGS="-arch i386 -arch x86_64 -headerpad_max_install_names -L/Library/Frameworks/SQLite3.framework/unix/lib -L/Library/Frameworks/UnixImageIO.framework/unix/lib -L/Library/Frameworks/PROJ.framework/unix/lib"
#export CFLAGS="-Os -arch i386 -arch x86_64 -I/Library/Frameworks/SQLite3.framework/unix/include -I/Library/Frameworks/UnixImageIO.framework/unix/include -I/Library/Frameworks/PROJ.framework/unix/include"
#export CXXFLAGS=$CFLAGS
#cd librasterlite
#./configure --disable-dependency-tracking --prefix=$PREFIX
#make clean
#make -j4
#make install
#cd ../../sources/lib
#install_name_tool -id $INSTALL/librasterlite.0.dylib librasterlite.0.dylib


### MAPNIK ###

# make sure we set DYLD path so we can link to libs without installing
export DYLD_LIBRARY_PATH=$PREFIX/lib

# compile mapnik using osx/config.py
scons PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig -j2 install BINDINGS=''

# then compile each python version..

# 2.5
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure BINDINGS=python PYTHON=/usr/bin/python2.5 BOOST_PYTHON_LIB=boost_python25
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_25.so

# 2.6
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure BINDINGS=python PYTHON=/usr/bin/python2.6 BOOST_PYTHON_LIB=boost_python26
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_26.so

# 2.7
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure BINDINGS=python PYTHON=/Library/Frameworks/Python.framework/Versions/2.7/bin/python2.7 BOOST_PYTHON_LIB=boost_python27
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_27.so


# 3.1
# needs patch: http://trac.mapnik.org/wiki/Python3k
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure BINDINGS=python PYTHON=/Library/Frameworks/Python.framework/Versions/3.1/bin/python3.1 BOOST_PYTHON_LIB=boost_python31
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_31.so

# 3.2
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure BINDINGS=python PYTHON=/Library/Frameworks/Python.framework/Versions/3.2/bin/python3.2m BOOST_PYTHON_LIB=boost_python32
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_32.so


# build a ton of versions of node (just to be safe about ABI)
cd ../../deps

for VER in {"0.2.4","0.2.5","0.2.6","0.3.0","0.3.1","0.3.2","0.3.3","0.3.4","0.3.5","0.3.6","0.3.7"}
do
  wget http://nodejs.org/dist/node-v$VER.tar.gz
  tar xvf node-v$VER.tar.gz
  cd node-v$VER
  ./configure --prefix=$PREFIX/node$VER
  make
  make install
  cd ../
done

# node-mapnik
cd ../../deps
git clone git://github.com/mapnik/node-mapnik.git
cd node-mapnik
#export PATH=../../Library/Frameworks/Mapnik.framework/Programs:$PATH
export PATH=/Library/Frameworks/Mapnik.framework/Programs:$PATH

# TODO - needs work
# only 64 bit
# versioned module
# all targets
# custom node prefix
# cairo support

#CXXFLAGS=" -g -DNDEBUG -O3 -Wall -DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE -ansi -finline-functions -Wno-inline -fPIC -arch x86_64 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DEV_MULTIPLICITY=0 -I/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/include -I/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/include/freetype2 "

#for VER in {"0.2.4","0.2.5","0.2.6","0.3.0","0.3.1","0.3.2","0.3.3","0.3.4"}
#for VER in {"0.3.4","0.3.5","0.3.6","0.3.7"}
#do
#  mkdir build/default/src/$VER
#  mkdir lib/$VER
#  NODE_PREFIX="$PREFIX/node$VER"
#  export PATH=$NODE_PREFIX/bin:$PATH
#  OBJ="build/default/src/$VER/_mapnik_1.o"
#  TARGET="lib/$VER/_mapnik.node"
#  g++ $CXXFLAGS -I$NODE_PREFIX/include/node src/_mapnik.cc -c -o $OBJ
#  LDFLAGS="-L/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/lib -lmapnik2 -bundle -#undefined dynamic_lookup"
#  g++ $OBJ -o $TARGET $LDFLAGS
#done


# then re-run wrap.py