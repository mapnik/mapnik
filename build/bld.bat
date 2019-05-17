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
-DBOOST_PREFIX=F:\wt.extensions\boost_1_65_1
)
echo done