@echo off
if not defined DevEnvDir (
 call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
 if errorlevel 1 goto :eof
)

set MSVC_VERSION="Visual Studio 16 2019"
echo using %MSVC_VERSION%

:: current place
set root_win=%cd%
set build=%root_win%
echo building in %build%


:: //////////////////////////////////////////////////////////
:: boost
:: //////////////////////////////////////////////////////////

:build_boost
if exist %build%\boost_1_65_1\stage\lib\libboost_filesystem-vc140-mt-gd-1_65_1.lib (
 echo skipping boost build
 goto build_icu
) else (
  echo building boost
  pushd boost_1_65_1
  bootstrap.bat
  b2 --prefix=. -j4 --with-system --with-regex --with-filesystem address-model=64 architecture=x86 variant=debug threading=multi link=static runtime-link=shared stage
  popd
  if errorlevel 1 goto :eof
)

:: //////////////////////////////////////////////////////////
:: icu
:: //////////////////////////////////////////////////////////

:build_icu
if exist %build%\icu4c-64_2\lib64\icuucd.lib (
 echo skipping ICU build
 goto build_free_type
) else (
  echo building ICU
  pushd icu4c-64_2
  msbuild source\allinone\allinone.sln /m /target:Build /property:Configuration=Debug;Platform=x64
  popd
  if errorlevel 1 goto :eof
)

:: //////////////////////////////////////////////////////////
:: free_type
:: //////////////////////////////////////////////////////////

:build_free_type
if exist %build%\freetype-2.10.0\objs\x64\Debug\freetype.lib (
 echo skipping free type
 goto build_libxml2
) else (
  echo building free type
  pushd freetype-2.10.0
  msbuild builds\windows\vc2010\freetype.sln /m /target:Build /property:Configuration=Debug;Platform=x64
  popd
  if errorlevel 1 goto :eof
)


:: //////////////////////////////////////////////////////////
:: libxml2
:: //////////////////////////////////////////////////////////

:build_libxml2
if exist %build%\libxml2-2.9.9\win32\VC10\x64\Debug\libxml2.lib (
 echo skipping libxml2
 goto build_libpng
) else (
  echo building libxml2
  pushd libxml2-2.9.9
  msbuild win32\VC10\libxml2.sln /m /target:Build /property:Configuration=Debug;Platform=x64
  popd
  if errorlevel 1 goto :eof
)

:: //////////////////////////////////////////////////////////
:: libpng
:: //////////////////////////////////////////////////////////

:build_libpng
if exist %build%\libpng-1.6.37\projects\vstudio\x64\Debug\libpng16.lib (
 echo skipping libpng
) else (
  echo building libpng
  pushd libpng-1.6.37
  msbuild projects\vstudio\vstudio.sln /m /target:Build /property:Configuration=Debug;Platform=x64
  popd
  if errorlevel 1 goto :eof
)