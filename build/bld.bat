:: bld.bat
:: Windows script to build Mapnik on the command line, using Visual Studio 
:: Pedro Vicente

@echo off
if not defined DevEnvDir (
 call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
 if errorlevel 1 goto :eof
)
set MSVC_VERSION="Visual Studio 14 2015 Win64"
echo using %MSVC_VERSION%

:build_mapnik
echo building Mapnik
rm -rf CMakeCache.txt CMakeFiles
cmake .. ^
-G %MSVC_VERSION% ^
-DBOOST_PREFIX=M:\mapnik\boost_1_65_1 ^
-Dlibboost_regex=M:\mapnik\boost_1_65_1\stage\lib\libboost_regex-vc140-mt-gd-1_65_1.lib ^
-Dlibboost_filesystem=M:\mapnik\boost_1_65_1\stage\lib\libboost_filesystem-vc140-mt-gd-1_65_1.lib ^
-Dlibboost_system=M:\mapnik\boost_1_65_1\stage\lib\libboost_system-vc140-mt-gd-1_65_1.lib ^
-DICU_INCLUDE=M:\mapnik\icu4c-64_2\source\common ^
-DLIB_ICU=M:\mapnik\icu4c-64_2\lib64\icuucd.lib ^
-DFREE_TYPE_INCLUDE=M:\mapnik\freetype-2.10.0\include ^
-DLIB_FREETYPE=M:\mapnik\freetype-2.10.0\objs\x64\Debug\freetype.lib ^
-DLIBXML2_INCLUDE=M:\mapnik\libxml2-2.9.9\include ^
-DLIB_LIBXML2=M:\mapnik\libxml2-2.9.9\win32\VC10\x64\Debug\libxml2.lib ^
-DLIBICONV_INCLUDE=M:\mapnik\libiconv-1.14\source\include ^
-DPNG_INCLUDES=M:\mapnik\libpng-1.6.37 ^
-DPNG_LIBS=M:\mapnik\libpng-1.6.37\projects\vstudio\x64\Debug\libpng16.lib

:: 
:: build
::

msbuild mapnik.sln /target:build /property:configuration=debug /nologo /verbosity:minimal
if errorlevel 1 goto :eof
echo done