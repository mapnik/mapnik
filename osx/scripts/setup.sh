
# build icu and boost for packaging up within Mapnik Framework

# local install location
PREFIX=/Users/dane/projects/mapnik-dev/trunk-build/osx/sources
mkdir /Users/dane/projects/mapnik-dev/trunk-build/osx/sources

# final resting place
INSTALL=/Library/Frameworks/Mapnik.framework/unix/lib

# make a directory to hold icu and boost
mkdir ../deps
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
  stage

./bjam --prefix=$PREFIX --with-python --with-thread --with-filesystem \
  --with-iostreams --with-regex \
  --with-program_options --with-system \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=32_64 \
  architecture=x86 \
  install

#cp stage/lib/libboost_*dylib ../../sources/lib/

cd ../../sources/lib

install_name_tool -id $INSTALL/libboost_python.dylib libboost_python.dylib
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
