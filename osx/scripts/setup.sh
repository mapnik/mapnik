
# start here
cd osx/sources

# build icu and boost for packaging up within Mapnik Framework

# local install location
PREFIX=/Users/dane/projects/mapnik-dev/trunk-build/osx/sources
mkdir -p /Users/dane/projects/mapnik-dev/trunk-build/osx/sources
export DYLD_LIBRARY_PATH=$PREFIX/lib


# final resting place
INSTALL=/Library/Frameworks/Mapnik.framework/unix/lib

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
./runConfigureICU MacOSX --prefix=$PREFIX --disable-static --enable-shared --disable-samples --disable-icuio --disable-layout --disable-tests --disable-extras --with-library-bits=64
make install -j4
# note -R is needed to preserve the symlinks
#cp -R lib/libicuuc.* ../../../sources/lib/
#cp -R lib/libicudata.* ../../../sources/lib/
cd ../../../sources/lib/


# libicuuc
install_name_tool -id $INSTALL/libicuuc.46.dylib libicuuc.46.0.dylib
install_name_tool -change ../lib/libicudata.46.0.dylib $INSTALL/libicudata.46.dylib libicuuc.46.0.dylib
# libicudata
install_name_tool -id $INSTALL/libicudata.46.dylib libicudata.46.0.dylib
# libicui18n - needed by boost_regex
install_name_tool -id $INSTALL/libicui18n.46.dylib libicui18n.46.0.dylib
install_name_tool -change ../lib/libicudata.46.0.dylib $INSTALL/libicudata.46.dylib libicui18n.46.0.dylib
install_name_tool -change libicuuc.46.dylib $INSTALL/libicuuc.46.dylib libicui18n.46.0.dylib

# boost
cd ../../deps
wget http://voxel.dl.sourceforge.net/project/boost/boost/1.45.0/boost_1_45_0.tar.bz2
tar xjvf boost_1_45_0.tar.bz2
cd boost_1_45_0

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
#--prefix-dir
./bjam --prefix=$PREFIX --with-python --with-thread --with-filesystem \
  --with-iostreams --with-regex \
  --with-program_options --with-system \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=32_64 \
  architecture=x86 \
  link=shared \
  stage

./bjam --prefix=$PREFIX --with-python --with-thread --with-filesystem \
  --with-iostreams --with-regex \
  --with-program_options --with-system \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=32_64 \
  architecture=x86 \
  link=shared \
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
cp stage/lib/libboost_python3.dylib ../../sources/lib/libboost_python31.dylib


#cp stage/lib/libboost_*dylib ../../sources/lib/

cd ../../sources/lib

# fix boost pythons
install_name_tool -id $INSTALL/libboost_python25.dylib libboost_python25.dylib
install_name_tool -id $INSTALL/libboost_python26.dylib libboost_python26.dylib
install_name_tool -id $INSTALL/libboost_python27.dylib libboost_python27.dylib
install_name_tool -id $INSTALL/libboost_python31.dylib libboost_python31.dylib

# fix boost libs
install_name_tool -id $INSTALL/libboost_system.dylib libboost_system.dylib
install_name_tool -id $INSTALL/libboost_filesystem.dylib libboost_filesystem.dylib
install_name_tool -id $INSTALL/libboost_regex.dylib libboost_regex.dylib
install_name_tool -id $INSTALL/libboost_program_options.dylib libboost_program_options.dylib
install_name_tool -id $INSTALL/libboost_iostreams.dylib libboost_iostreams.dylib
install_name_tool -id $INSTALL/libboost_thread.dylib libboost_thread.dylib
install_name_tool -change libboost_system.dylib $INSTALL/libboost_system.dylib libboost_filesystem.dylib
#install_name_tool -change libicui18n.46.dylib $INSTALL/libicui18n.46.dylib libboost_regex.dylib

# rasterlite we must bundle as it is not available in the SQLite.framework
cd ../../deps
svn co https://www.gaia-gis.it/svn/librasterlite
export LDFLAGS="-arch i386 -arch x86_64 -headerpad_max_install_names -L/Library/Frameworks/SQLite3.framework/unix/lib -L/Library/Frameworks/UnixImageIO.framework/unix/lib -L/Library/Frameworks/PROJ.framework/unix/lib"
export CFLAGS="-Os -arch i386 -arch x86_64 -I/Library/Frameworks/SQLite3.framework/unix/include -I/Library/Frameworks/UnixImageIO.framework/unix/include -I/Library/Frameworks/PROJ.framework/unix/include"
export CXXFLAGS=$CFLAGS
cd librasterlite
./configure --disable-dependency-tracking --prefix=$PREFIX
make clean
make -j4
make install

cd ../../sources/lib

install_name_tool -id $INSTALL/librasterlite.0.dylib librasterlite.0.dylib

# freetype2
cd ../../deps
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.4.tar.gz
tar xvf freetype-2.4.4.tar.gz
export CFLAGS="-O3 -arch i386 -arch x86_64"
export LDFLAGS="-arch i386 -arch x86_64 -headerpad_max_install_names"
cd freetype-2.4.4
./configure --prefix=$PREFIX
make -j4
make install
cd ../../sources/lib
install_name_tool -id $INSTALL/libfreetype.6.dylib libfreetype.6.dylib

### MAPNIK ###

# make sure we set DYLD path so we can link to libs without installing
export DYLD_LIBRARY_PATH=$PREFIX/lib

# compile mapnik using osx/config.py


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
scons configure PYTHON=/usr/bin/python2.6 BOOST_PYTHON_LIB=boost_python26
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_26.so

# 2.7
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure PYTHON=/Library/Frameworks/Python.framework/Versions/2.7/bin/python2.7 BOOST_PYTHON_LIB=boost_python27
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_27.so


# 3.1
# needs patch: http://trac.mapnik.org/wiki/Python3k
rm bindings/python/*os
rm bindings/python/mapnik/_mapnik2.so
scons configure PYTHON=/Library/Frameworks/Python.framework/Versions/3.1/bin/python3.1 BOOST_PYTHON_LIB=boost_python31
scons -j2 install
cp bindings/python/mapnik/_mapnik2.so osx/python/_mapnik2_31.so


# build a ton of versions of node (just to be safe about ABI)
cd ../../deps

for VER in {"0.2.4","0.2.5","0.2.6","0.3.0","0.3.1","0.3.2","0.3.3"}
do
  wget http://nodejs.org/dist/node-v$VER.tar.gz
  tar xvf node-v$VER.tar.gz
  cd node-v$VER
  ./configure --prefix=$PREFIX/node$VER
  make
  make install
  cd ../
done

# HEAD (master)
VER="0.3.4"
git clone git://github.com/ry/node.git node-master
cd node-master
./configure --prefix=$PREFIX/node$VER
make
make install
cd ../

# node-mapnik
cd ../../deps
git clone git://github.com/mapnik/node-mapnik.git
cd node-mapnik
export PATH=../../Library/Frameworks/Mapnik.framework/Programs:$PATH

CXXFLAGS=" -g -DNDEBUG -O3 -Wall -DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE -ansi -finline-functions -Wno-inline -fPIC -arch x86_64 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DEV_MULTIPLICITY=0 -I/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/include -I/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/include/freetype2 "

for VER in {"0.2.4","0.2.5","0.2.6","0.3.0","0.3.1","0.3.2","0.3.3","0.3.4"}
for VER in {"0.3.4",}
do
  mkdir build/default/src/$VER
  mkdir mapnik/$VER
  NODE_PREFIX="$PREFIX/node$VER"
  export PATH=$NODE_PREFIX/bin:$PATH
  OBJ="build/default/src/$VER/_mapnik_1.o"
  TARGET="mapnik/$VER/_mapnik.node"
  g++ $CXXFLAGS -I$NODE_PREFIX/include/node src/_mapnik.cc -c -o $OBJ
  LDFLAGS="-L/Library/Frameworks/Mapnik.framework/Versions/2.0/unix/lib -lmapnik2 -bundle -undefined dynamic_lookup"
  g++ $OBJ -o $TARGET $LDFLAGS
done


# then re-run wrap.py