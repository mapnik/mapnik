:: bld.bat
:: Windows script to build Mapnik on the command line, using Visual Studio 
:: Pedro Vicente

@echo off
set MSVC_VERSION="Visual Studio 14 2015 Win64"
echo using %MSVC_VERSION%

:build_mapnik
echo building Mapnik
rm -rf CMakeCache.txt CMakeFiles
cmake .. ^
-G %MSVC_VERSION% ^
-DBOOST_PREFIX=F:\wt.extensions\boost_1_65_1 ^
-DICU_INCLUDE=L:\icu\source\common ^
-DFREE_TYPE_INCLUDE=L:\freetype-2.10.0\include ^
-DZLIB_INCLUDE=L:\zlib-1.2.11 ^
-DLIBXML2_INCLUDE=L:\libxml2-2.9.9\include ^
-DLIBICONV_INCLUDE=L:\libiconv-1.14\source\include
)
echo done