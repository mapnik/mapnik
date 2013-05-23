@echo off

set MAPNIK_VERSION=v2.2.0
set MAPNIK_PREFIX=c:\\mapnik-%MAPNIK_VERSION%
set MAPNIK_LIBS=%MAPNIK_PREFIX%\lib
set MAPNIK_INCLUDES=%MAPNIK_PREFIX%\include
set MAPNIK_INPUT_PLUGINS_DIRECTORY=%MAPNIK_PREFIX%\input
set MAPNIK_FONTS_DIRECTORY=%MAPNIK_PREFIX%\fonts

if /i "%1"=="" goto help
if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help
if /i "%1"=="--prefix" echo %MAPNIK_PREFIX%
if /i "%1"=="--libs" echo mapnik.lib
@rem TODO - figure out how to avoid hardcoding these library names
if /i "%1"=="--dep-libs" echo icuuc.lib icuin.lib libboost_system-vc100-mt-s-1_49.lib
if /i "%1"=="--ldflags" echo %MAPNIK_LIBS%
if /i "%1"=="--defines" echo _WINDOWS HAVE_JPEG HAVE_PNG HAVE_TIFF MAPNIK_USE_PROJ4 BOOST_REGEX_HAS_ICU MAPNIK_THREADSAFE BIGINT HAVE_LIBXML2 HAVE_CAIRO
@rem /MD is multithreaded dynamic linking - http://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
@rem /EHsc is to support c++ exceptions - http://msdn.microsoft.com/en-us/library/1deeycx5(v=vs.80).aspx
@rem /GR is to support rtti (runtime type detection) - http://msdn.microsoft.com/en-us/library/we6hfdy0.aspx
if /i "%1"=="--cxxflags" echo /MD /EHsc /GR
if /i "%1"=="--includes" echo %MAPNIK_INCLUDES%
if /i "%1"=="--input-plugins" echo %MAPNIK_INPUT_PLUGINS_DIRECTORY%
if /i "%1"=="--fonts" echo %MAPNIK_FONTS_DIRECTORY%
goto exit


:help
echo mapnik-config.bat
echo Examples:
echo   --libs            : provide lib name for mapnik.dll
echo   --defines         : provide compiler defines needed for this mapnik build
echo   --dep-libs        : provide lib names of depedencies
echo   --ldflags         : provide lib paths to depedencies
echo   --cxxflags        : provide compiler flags
echo   --includes        : provide header paths for mapnik
echo   --dep-includes    : provide header paths for dependencies
echo   --input-plugins   : provide path to input plugins directory
echo   --fonts           : provide path to fonts directory
goto exit

:exit
goto :EOF
