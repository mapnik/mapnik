
PREFIX=/Users/dane/projects/mapnik-dev/trunk-build-static/osx/sources
mkdir -p $PREFIX
export DYLD_LIBRARY_PATH=$PREFIX/lib
# final resting place
INSTALL=/Library/Frameworks/Mapnik.framework/unix/lib
export DYLD_LIBRARY_PATH=$PREFIX/lib
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
export PATH=$PREFIX/bin:$PATH
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"


cd osx
mkdir -p deps
cd deps


wget http://download.icu-project.org/files/icu4c/4.6.1/icu4c-4_6_1-src.tgz
tar xvf icu4c-4_6_1-src.tgz
cd icu/source

# universal flags
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
./runConfigureICU MacOSX --prefix=$PREFIX --enable-static --disable-shared \
--with-library-bits=64 --enable-release \
--with-data-packaging=static

#Data Packaging: archive
# This means: ICU data will be stored in a single .dat file.
# To locate data: ICU will look in /Users/dane/projects/mapnik-dev/trunk-build-static/osx/sources/share/icu/4.6 which is the #installation location. Call u_setDataDirectory() or use the ICU_DATA environment variable to override.

make install -j4

ln -s `pwd`/../sources/lib `pwd`/../sources/lib64



wget http://voxel.dl.sourceforge.net/project/boost/boost/1.47.0/boost_1_47_0.tar.bz2
tar xjvf boost_1_47_0.tar.bz2
cd boost_1_47_0
./bootstrap.sh
#--prefix-dir

#   architecture=x86  prevents icu link test from working?
l
./bjam --prefix=$PREFIX --with-python \
  --with-thread \
  --with-filesystem \
  --with-program_options --with-system --with-chrono \
  --with-regex define=U_STATIC_IMPLEMENTATION=1 \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=64 \
  link=static \
  variant=release \
  stage

./bjam --prefix=$PREFIX --with-python \
  --with-thread \
  --with-filesystem \
  --with-program_options --with-system --with-chrono \
  --with-regex define=U_STATIC_IMPLEMENTATION=1 \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=64 \
  link=static \
  variant=release \
  install

# no icu variant
./bjam --prefix=$PREFIX --with-python \
  --with-thread \
  --with-filesystem \
  --with-program_options --with-system --with-chrono \
  --with-regex --disable-icu \
  toolset=darwin \
  address-model=64 \
  link=static \
  variant=release \
  install


./bjam --prefix=$PREFIX -a \
  --with-regex define=U_STATIC_IMPLEMENTATION=1 \
  -sHAVE_ICU=1 -sICU_PATH=$PREFIX \
  toolset=darwin \
  address-model=64 \
  link=static \
  variant=release \
  stage

cp stage/lib/libboost_regex.a ../../sources/lib/


tar xvf ../deps/freetype-2.4.4.tar.gz
export CFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
cd freetype-2.4.4
./configure --prefix=$PREFIX \
--enable-static \
--disable-shared
make -j4
make install



wget http://download.osgeo.org/proj/proj-4.7.0.tar.gz
tar xvf proj-4.7.0.tar.gz
cd proj-4.7.0
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make
make install


wget http://downloads.sourceforge.net/project/libpng/libpng12/1.2.44/libpng-1.2.44.tar.bz2
tar xvf libpng-1.2.44.tar.bz2
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install

wget http://www.ijg.org/files/jpegsrc.v8c.tar.gz
tar xvf jpegsrc.v8c.tar.gz
cd jpeg-8c
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install

wget http://download.osgeo.org/libtiff/tiff-3.9.4.zip
tar xvf tiff-3.9.4.zip
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
cd tiff-3.9.4
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking
make -j4
make install


# gdal 1.8
./configure --prefix=$PREFIX --disable-static --enable-shared --disable-dependency-tracking \

export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
./configure --prefix=$PREFIX --enable-static --disable-shared --disable-dependency-tracking \
--with-libtiff=$PREFIX \
--with-jpeg=$PREFIX \
--with-png=$PREFIX \
--with-geos=no \
--with-pcraster=no \
--with-libkml=no \
--with-pcidsk=no \
--with-jasper=no \
--with-gif=no \
--with-pg=no 

make -j4

file ../../sources/lib/libgdal.a

ldd ../../sources/lib/libgdal.dylib


# cairo and friends

# pkg-config so we get cairo and friends configured correctly
wget http://pkgconfig.freedesktop.org/releases/pkg-config-0.26.tar.gz
tar xvf pkg-config-0.26.tar.gz
cd pkg-config-0.26
./configure --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

wget http://cairographics.org/releases/pixman-0.22.2.tar.gz
tar xvf pixman-0.22.2.tar.gz
cd pixman-0.22.2
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

wget http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.8.0.tar.gz
tar xvf fontconfig-2.8.0.tar.gz
cd fontconfig-2.8.0
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX \
    --with-freetype-config=$PREFIX/bin/freetype-config
make -j4
make install
cd ../


# Cairo
wget http://cairographics.org/releases/cairo-1.10.2.tar.gz
tar xvf cairo-1.10.2.tar.gz
cd cairo-1.10.2
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export LDFLAGS="-L$PREFIX/lib "$LDFLAGS
export CFLAGS="-I$PREFIX/include "$CFLAGS
export png_CFLAGS="-I$PREFIX/include"
export png_LIBS="-I$PREFIX/lib -lpng12"
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
./configure --enable-static --disable-shared --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
cd ../

wget http://cairographics.org/releases/cairomm-1.10.0.tar.gz
tar xvf cairomm-1.10.0.tar.gz
cd cairomm-1.10.0
# NOTE: PKG_CONFIG_PATH must be correctly set by this point
export CFLAGS="-O3 -arch x86_64"
export CXXFLAGS="-O3 -arch x86_64"
export LDFLAGS="-arch x86_64"
export LDFLAGS="-L$PREFIX/lib -lcairo -lfontconfig -lsigc-2.0 "$LDFLAGS
export CFLAGS="-I$PREFIX/include -I$PREFIX/include/cairo -I$PREFIX/include/freetype2 -I$PREFIX/include/fontconfig -I$PREFIX/lib/sigc++-2.0/include -I$PREFIX/include/sigc++-2.0 -I$PREFIX/include/sigc++-2.0/sigc++ "$CFLAGS
# undef CAIRO_HAS_FT_FONT in include/cairo/cairo-features.h
#mv ../../sources/lib/pkgconfig/cairo-ft.pc ../../sources/lib/pkgconfig/_cairo-ft.pc
#export CFLAGS="-DNCAIRO_HAS_FT_FONT -I$PREFIX/include -I$PREFIX/include/cairo -I$PREFIX/lib/sigc++-2.0/include -I$PREFIX/include/sigc++-2.0 -I$PREFIX/include/sigc++-2.0/sigc++ "
export CXXFLAGS="-I$PREFIX/include "$CFLAGS

./configure --enable-static --disable-shared \
    --disable-dependency-tracking --prefix=$PREFIX
make -j4
make install
