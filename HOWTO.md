### Initial setup


```bash
export MASON_DIR="<path-to-mason-dir>"
```

### Building gdal (non-sanitized build)
```bash
echo "Configuring..."
CC="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang" \
CXX="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++ -std=c++14" \
CXXFLAGS="-nostdinc++ -I$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/include/c++/v1 -I/opt/mapnik_static_deps/include" LDFLAGS="-stdlib=libc++ -nostdlib++ $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++abi.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libunwind.a -rtlib=compiler-rt -L/opt/mapnik_static_deps/lib" ./configure --without-libtool --with-geos=no --with-curl=no --with-threads=yes --with-hide-internal-symbols=yes --with-xml2=no --with-pcraster=no --with-cfitsio=no --with-odbc=no --with-libkml=no --with-pcidsk=no --with-jasper=no --with-gif=no --with-grib=no --with-freexl=no --with-avx=no --with-sse=no --with-perl=no --with-python=no --with-java=no --with-podofo=no --with-pam  --with-webp=no --with-pcre=no --with-liblzma=no --with-netcdf=no --with-poppler=no --with-sfcgal=no --with-fgdb=no --prefix=/opt/mapnik_static_deps --enable-static=yes --enable-shared=no
echo "Buiding..."
make -j4
echo "Done!"

## cd /usr/include
## sudo ln -s locale.h xlocale.h
```

### Building ICU 60.1

echo "Configuring ICU with c++14/libc++ ..."

```bash
CC="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang" \
CXX="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++" \
LD="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++" \
CXXFLAGS="-fPIC -nostdinc++ -I$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/include/c++/v1 -I/opt/mapnik_static_deps/include" \
  LDFLAGS="-fPIC -stdlib=libc++ -nostdlib++  -rtlib=compiler-rt -ldl -pthread -L/opt/mapnik_static_deps/lib" \
  LIBS="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++abi.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libunwind.a" \
  FORCE_LIB=1 \
  ./runConfigureICU "Linux" \
  --with-data-packaging=archive \
  --prefix=/opt/mapnik_static_deps \
  --enable-strict \
  #--enable-static \
  --enable-draft \
  --disable-rpath \
  --disable-shared \
  --disable-tests \
  --disable-extras \
  --disable-tools \
  --disable-tracing \
  --disable-layout \
  --disable-icuio \
  --disable-samples \
  #--disable-dyload

echo "Building ..."
make -j4 VERBOSE=1
echo "Done!"
```

### Building boost 1_66
```bash
echo "Building and installing Boost libraries with c++14/libc++ ..."
CC="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++" \
CXX="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++" \
LD="$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++" \
./b2 -sICU_PATH="/opt/mapnik_static_deps" include="/opt/mapnik_static_deps/include" toolset=clang cxxflags="-std=c++14 -nostdinc++ -I$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/include/c++/v1" linkflags="-stdlib=libc++ -nostdlib++ $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libc++abi.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libunwind.a -rtlib=compiler-rt -L/opt/mapnik_static_deps/lib -pthread -ldl" --prefix=/opt/boost_1_66_sanitized --with-iostreams --with-filesystem --with-regex --with-python --with-thread --with-program_options --with-system  install
echo "Done!"
```

### Building Mapnik 
Sample config.py


```python
CXX = '$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang++'
CC = '$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/bin/clang'
CUSTOM_CXXFLAGS = ' -fsanitize=memory -nostdinc++ -I$MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/include/c++/v1'
CUSTOM_LDFLAGS = '-fsanitize=memory -stdlib=libc++ -nostdlib++ $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/msan/lib/libc++.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/msan/lib/libc++abi.a $MASON_DIR/mason_packages/linux-x86_64/llvm/7.0.0/lib/libunwind.a -rtlib=compiler-rt -ldl -pthread'
INPUT_PLUGINS = 'gdal,geojson,ogr,raster,shape,sqlite'
PREFIX = '/opt/mapnik-sanitized'
BOOST_INCLUDES = '/opt/boost_1_66_sanitized/include'
BOOST_LIBS = '/opt/boost_1_66_sanitized/lib'
ICU_INCLUDES = '/opt/mapnik_static_deps/include'
ICU_LIBS = '/opt/mapnik_static_deps/lib'
SVG_RENDERER = True
CAIRO_INCLUDES = '/opt/mapnik_static_deps/include'
CAIRO_LIBS = '/opt/mapnik_static_deps/lib'
```

