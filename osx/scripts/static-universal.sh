# build notes for compiling mapnik deps statically
# and "FAT" (aka. universal) in order to allow
# linking a fully standalone libmapnik.a

PREFIX=`pwd`/osx/sources
mkdir -p $PREFIX
export DYLD_LIBRARY_PATH=$PREFIX/lib
export DYLD_LIBRARY_PATH=$PREFIX/lib
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
export PATH=$PREFIX/bin:$PATH
export CORE_CFLAGS="-O3 -arch x86_64 -arch i386 -mmacosx-version-min=10.6 -isysroot /Developer/SDKs/MacOSX10.6.sdk"
export CORE_CXXFLAGS=$CORE_CFLAGS
export CORE_LDFLAGS="-Wl,-search_paths_first -arch x86_64 -arch i386 -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk"
export ARCHFLAGS="-arch x86_64 -arch i386"


cd osx
mkdir -p deps
cd deps


# icucore headers - temporary workaround until I can get more recent icu versions working
wget http://www.opensource.apple.com/tarballs/ICU/ICU-400.38.tar.gz
tar xvf ICU-400.38.tar.gz
cd ICU-400.38
make install
cp -R build/usr/local/include/unicode/ ../../sources/include/unicode
cd ../

# boost
wget http://voxel.dl.sourceforge.net/project/boost/boost/1.47.0/boost_1_47_0.tar.bz2
tar xjvf boost_1_47_0.tar.bz2
cd boost_1_47_0
./bootstrap.sh

# no icu variant
./bjam --prefix=$PREFIX -j2 -d2 \
  --with-thread \
  --with-filesystem \
  --with-program_options --with-system --with-chrono \
  --with-regex --disable-icu \
  toolset=darwin \
  macosx-version=10.6 \
  address-model=32_64 \
  architecture=x86 \
  link=static \
  variant=release \
  stage

./bjam --prefix=$PREFIX -j2 -d2 \
  --with-thread \
  --with-filesystem \
  --with-program_options --with-system --with-chrono \
  --with-regex --disable-icu \
  toolset=darwin \
  macosx-version=10.6 \
  address-model=32_64 \
  architecture=x86 \
  link=static \
  variant=release \
  install

cd ../

# universal flags
export CFLAGS=$CORE_CFLAGS
export CXXFLAGS=$CORE_CXXFLAGS
export LDFLAGS=$CORE_LDFLAGS

# sqlite
wget http://www.sqlite.org/sqlite-autoconf-3070701.tar.gz
tar xvf sqlite-autoconf-3070701.tar.gz
cd sqlite-autoconf-3070701
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../


# freetype
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.6.tar.bz2
tar xvf freetype-2.4.6.tar.bz2
cd freetype-2.4.6
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../

# proj4
wget http://download.osgeo.org/proj/proj-datumgrid-1.5.zip
#wget http://download.osgeo.org/proj/proj-4.7.0.tar.gz
#tar xvf proj-4.7.0.tar.gz
#cd proj-4.7.0
# we use trunk instead for better threading support
svn co http://svn.osgeo.org/metacrs/proj/trunk/proj proj-trunk # at the time pre-release 4.8.0
cd proj-trunk/nad
unzip ../../proj-datumgrid-1.5.zip
cd ../
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../

# libpng
wget ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.5.4.tar.gz
tar xvf libpng-1.5.4.tar.gz
cd libpng-1.5.4
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../

# libjpeg
wget http://www.ijg.org/files/jpegsrc.v8c.tar.gz
tar xvf jpegsrc.v8c.tar.gz
cd jpeg-8c
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../

# libtiff
wget http://download.osgeo.org/libtiff/tiff-3.9.5.tar.gz
tar xvf tiff-3.9.5.tar.gz
cd tiff-3.9.5
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install
cd ../

# gettext which provides libintl for libpq
wget http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz
tar xvf gettext-0.18.1.1.tar.gz
cd gettext-0.18.1.1
export CC=gcc
export CXX=g++
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking \
--without-included-gettext --disable-debug --without-included-glib \
--without-included-libcroco  --without-included-libxml \
--without-emacs --without-git --without-cvs
make -j4
make install
cd ../


# postgis
# install postgres 9.x client from somewhere. 
# http://www.kyngchaos.com/software/postgres works nicely
# then copy the clients static libpq.a to a prioritized directory:
cp /usr/local/pgsql/lib/libpq.a osx/sources/lib/libpq.a
# then edit plugins/input/postgis/SConscript adding more libs to link to


# gdal 1.8.1
wget http://download.osgeo.org/gdal/gdal-1.8.1.tar.gz
tar xvf gdal-1.8.1.tar.gz
cd gdal-1.8.1
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking \
--with-libtiff=$PREFIX \
--with-jpeg=$PREFIX \
--with-png=$PREFIX \
--with-static-proj4=$PREFIX \
--with-sqlite3=no \
--with-spatialite=no \
--with-curl=no \
--with-geos=no \
--with-pcraster=no \
--with-cfitsio=no \
--with-odbc=no \
--with-libkml=no \
--with-pcidsk=no \
--with-jasper=no \
--with-gif=no \
--with-pg=no \
--with-hide-internal-symbols=yes \
--with-vfk=no \
--with-grib=no

make -j4
make install
cd ../


# cairo and friends

# pkg-config so we get cairo and friends configured correctly
wget http://pkgconfig.freedesktop.org/releases/pkg-config-0.26.tar.gz
tar xvf pkg-config-0.26.tar.gz
cd pkg-config-0.26
./configure --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

# pixman
wget http://cairographics.org/releases/pixman-0.22.2.tar.gz
tar xvf pixman-0.22.2.tar.gz
cd pixman-0.22.2
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

# fontconfig
wget http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.8.0.tar.gz
tar xvf fontconfig-2.8.0.tar.gz
cd fontconfig-2.8.0
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX \
    --with-freetype-config=$PREFIX/bin/freetype-config
make -j4
make install
cd ../


# cairo
wget http://cairographics.org/releases/cairo-1.10.2.tar.gz
tar xvf cairo-1.10.2.tar.gz
cd cairo-1.10.2
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export LDFLAGS="-L$PREFIX/lib "$CORE_LDFLAGS
export CFLAGS="-I$PREFIX/include "$CORE_CFLAGS
export png_CFLAGS="-I$PREFIX/include"
export png_LIBS="-I$PREFIX/lib -lpng"
./configure \
  --enable-static --disable-shared \
  --enable-pdf=yes \
  --enable-ft=yes \
  --enable-png=yes \
  --enable-svg=yes \
  --enable-ps=yes \
  --enable-fc=yes \
  --enable-trace=no \
  --enable-gtk-doc=no \
  --enable-qt=no \
  --enable-quartz=no \
  --enable-quartz-font=no \
  --enable-quartz-image=no \
  --enable-win32=no \
  --enable-win32-font=no \
  --enable-skia=no \
  --enable-os2=no \
  --enable-beos=no \
  --enable-drm=no \
  --enable-drm-xr=no \
  --enable-gallium=no \
  --enable-gl=no \
  --enable-directfb=no \
  --enable-vg=no \
  --enable-egl=no \
  --enable-glx=no \
  --enable-wgl=no \
  --enable-test-surfaces=no \
  --enable-tee=no \
  --enable-xml=no \
  --enable-interpreter=no \
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
cd ../


# libsigcxx
wget http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.2/libsigc++-2.2.10.tar.bz2
tar xvf libsigc++-2.2.10.tar.bz2
cd libsigc++-2.2.10
export CFLAGS=$CORE_CFLAGS
export CXXFLAGS=$CORE_CXXFLAGS
export LDFLAGS=$CORE_LDFLAGS
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

# cairomm
wget http://cairographics.org/releases/cairomm-1.10.0.tar.gz
tar xvf cairomm-1.10.0.tar.gz
cd cairomm-1.10.0
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export LDFLAGS="-L$PREFIX/lib -lcairo -lfontconfig -lsigc-2.0 "$CORE_LDFLAGS
export CFLAGS="-I$PREFIX/include -I$PREFIX/include/cairo -I$PREFIX/include/freetype2 -I$PREFIX/include/fontconfig -I$PREFIX/lib/sigc++-2.0/include -I$PREFIX/include/sigc++-2.0 -I$PREFIX/include/sigc++-2.0/sigc++ "$CORE_CFLAGS
export CXXFLAGS="-I$PREFIX/include "$CFLAGS

./configure --enable-static --disable-shared \
    --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

# mapnik

# config.py

CXX = 'clang++'
CC = 'clang++'
CUSTOM_CXXFLAGS = '-arch x86_64 -arch i386 -mmacosx-version-min=10.6 -isysroot /Developer/SDKs/MacOSX10.6.sdk'
CUSTOM_LDFLAGS = '-Wl,-search_paths_first -arch x86_64 -arch i386 -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk'
INPUT_PLUGINS = 'gdal,ogr,postgis,shape'
DESTDIR = './osx/sources/'
BOOST_INCLUDES = './osx/sources/include'
BOOST_LIBS = './osx/sources/lib'
FREETYPE_CONFIG = './osx/sources/bin/freetype-config'
ICU_INCLUDES = './osx/sources/include'
ICU_LIB_NAME = 'icucore'
PNG_INCLUDES = './osx/sources/include'
PNG_LIBS = './osx/sources/lib'
JPEG_INCLUDES = './osx/sources/include'
JPEG_LIBS = './osx/sources/lib'
TIFF_INCLUDES = './osx/sources/include'
TIFF_LIBS = './osx/sources/lib'
PROJ_INCLUDES = './osx/sources/include'
PROJ_LIBS = './osx/sources/lib'
PKG_CONFIG_PATH = './osx/sources/lib/pkgconfig'
SQLITE_INCLUDES = './osx/sources/include'
SQLITE_LIBS = './osx/sources/lib'
BINDINGS = 'none'

