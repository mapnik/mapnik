@echo off

set MAPNIK_VERSION=2.3.0
set MAPNIK_VERSION_NUMBER=200300
::SET CUR_PATH=%CD%
::cd %CD%/../
set MAPNIK_PREFIX=%~dp0
:: strip trailing \
set MAPNIK_PREFIX=%MAPNIK_PREFIX:~0,-1%
:: get dirname
for %%F in (%MAPNIK_PREFIX%) do set MAPNIK_PREFIX=%%~dpF
:: strip trailing \
set MAPNIK_PREFIX=%MAPNIK_PREFIX:~0,-1%
:: now make double \\ for gyp
set MAPNIK_PREFIX=%MAPNIK_PREFIX:\=\\%
set MAPNIK_LIBS=%MAPNIK_PREFIX%\\libs
set MAPNIK_INCLUDES=%MAPNIK_PREFIX%\\includes
set MAPNIK_INPUT_PLUGINS_DIRECTORY=%MAPNIK_PREFIX%\\libs\\mapnik\\input
set MAPNIK_FONTS_DIRECTORY=%MAPNIK_PREFIX%\\libs\\mapnik\\fonts

if /i "%1"=="" (
  goto help_msg_err
)

:Loop
IF "%1"=="" GOTO Continue

    if /i "%1"=="-v" (
      echo %MAPNIK_VERSION%
      goto exit_ok
    )

    if /i "%1"=="--version" (
      echo %MAPNIK_VERSION%
      goto exit_ok
    )

    if /i "%1"=="--version-number" (
      echo %MAPNIK_VERSION_NUMBER%
      goto exit_ok
    )

    if /i "%1"=="--git-revision" (
      echo TODO
      goto exit_ok
    )

    if /i "%1"=="--git-describe" (
      echo TODO
      goto exit_ok
    )

    if /i "%1"=="help" (
      goto help_msg
    )

    if /i "%1"=="--help" (
      goto help_msg
    )

    if /i "%1"=="-help" (
      goto help_msg
    )

    if /i "%1"=="-h" (
      goto help_msg
    )

    if /i "%1"=="/help" (
      goto help_msg
    )

    if /i "%1"=="?" (
      goto help_msg
    )

    if /i "%1"=="-?" (
      goto help_msg
    )

    if /i "%1"=="--?" (
      goto help_msg
    )

    if /i "%1"=="/?" (
      goto help_msg
    )

    set hit=""

    if /i "%1"=="--prefix" (
      echo %MAPNIK_PREFIX%
      set hit=%1
    )

    if /i "%1"=="--input-plugins" (
      echo %MAPNIK_INPUT_PLUGINS_DIRECTORY%
      set hit=%1
    )

    if /i "%1"=="--fonts" (
      echo %MAPNIK_FONTS_DIRECTORY%
      set hit=%1
    )

    if /i "%1"=="--lib-name" (
      echo mapnik
      set hit=%1
    )

    if /i "%1"=="--libs" (
      echo mapnik.lib
      set hit=%1
    )

    @rem TODO - figure out how to avoid hardcoding these library names
    if /i "%1"=="--dep-libs" (
      echo libpng16.lib zlib.lib harfbuzz.lib libwebp_dll.lib libjpeg.lib icuuc.lib icuin.lib cairo.lib libboost_system-vc140-mt-1_56.lib libxml2_a.lib ws2_32.lib
      set hit=%1
    )

    if /i "%1"=="--ldflags" (
      echo %MAPNIK_LIBS%
      set hit=%1
    )

    if /i "%1"=="--defines" (
      echo _WINDOWS BOOST_ALL_NO_LIB BOOST_LIB_TOOLSET="vc140" BOOST_COMPILER="14.0" BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES HAVE_JPEG HAVE_PNG HAVE_WEBP HAVE_TIFF MAPNIK_USE_PROJ4 BOOST_REGEX_HAS_ICU GRID_RENDERER SVG_RENDERER MAPNIK_THREADSAFE BIGINT HAVE_LIBXML2 HAVE_CAIRO LIBXML_STATIC
      set hit=%1
    )

    @rem /MD is multithreaded dynamic linking - http://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
    @rem /EHsc is to support c++ exceptions - http://msdn.microsoft.com/en-us/library/1deeycx5(v=vs.80).aspx
    @rem /GR is to support rtti (runtime type detection) - http://msdn.microsoft.com/en-us/library/we6hfdy0.aspx
    if /i "%1"=="--cxxflags" (
      echo /MD /EHsc /GR
      set hit=%1
    )

    if /i "%1"=="--includes" (
      echo %MAPNIK_INCLUDES% %MAPNIK_INCLUDES%\\mapnik\\agg
      set hit=%1
    )

    if /i "%1"=="--all-flags" (
      @rem nothing here yet
      echo ""
      set hit=%1
    )

    if /i "%1"=="--cxx" (
      @rem nothing here yet
      echo ""
      set hit=%1
    )

    if /i "%1"=="--cflags" (
      @rem nothing here yet
      echo ""
      set hit=%1
    )

    if /i "%1"=="--dep-includes" (
      @rem nothing here yet
      echo %MAPNIK_INCLUDES%\\cairo %MAPNIK_INCLUDES%\\freetype2 %MAPNIK_INCLUDES%\\google %MAPNIK_INCLUDES%\\libxml2 
      set hit=%1
    )

    @rem if we got here print warning
    if /i NOT %1==%hit% (
      echo unknown option %1 1>&2
    )
SHIFT
GOTO Loop
:Continue


goto exit_ok

:help_msg
echo Usage: mapnik-config
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
goto exit_ok

:help_msg_err
echo Usage: mapnik-config
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
goto exit_error

:exit_error
exit /b 1
goto :EOF

:exit_ok
exit /b 0
goto :EOF