@echo off

::git clone https://chromium.googlesource.com/external/gyp.git
::CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
::SET PATH=C:\Python27;%PATH%

::ddt ..\mapnik-sdk
::ddt build\Release

if NOT EXIST gyp (
    CALL git clone https://chromium.googlesource.com/external/gyp.git gyp
    IF ERRORLEVEL 1 GOTO ERROR
)

:: run find command and bail on error
:: this ensures we have the unix find command on path
:: before trying to run gyp
find deps/clipper/src/ -name "*.cpp"
IF ERRORLEVEL 1 GOTO ERROR

CALL gyp\gyp.bat mapnik.gyp --depth=. ^
 -Dincludes=%CD%/../mapnik-sdk/includes ^
 -Dlibs=%CD%/../mapnik-sdk/libs ^
 -f msvs -G msvs_version=2013 ^
 --generator-output=build ^
 --no-duplicate-basename-check
IF ERRORLEVEL 1 GOTO ERROR

if NOT EXIST ..\mapnik-sdk (
  mkdir ..\mapnik-sdk
  mkdir ..\mapnik-sdk\includes
  mkdir ..\mapnik-sdk\share
  mkdir ..\mapnik-sdk\libs
  mkdir ..\mapnik-sdk\
)
IF ERRORLEVEL 1 GOTO ERROR

SET DEPSDIR=..

:: includes
xcopy /i /d /s /q %DEPSDIR%\boost_1_55_0\boost ..\mapnik-sdk\includes\boost /Y
xcopy /i /d /s /q %DEPSDIR%\icu\include\unicode ..\mapnik-sdk\includes\unicode /Y
xcopy /i /d /s /q %DEPSDIR%\freetype\include ..\mapnik-sdk\includes\freetype2 /Y
xcopy /i /d /s /q %DEPSDIR%\libxml2\include\libxml ..\mapnik-sdk\includes\libxml /Y
xcopy /i /d /s /q %DEPSDIR%\zlib-1.2.5\zlib.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\zlib-1.2.5\zconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libpng\png.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libpng\pnglibconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libpng\pngconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\jpeg\jpeglib.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\jpeg\jconfig.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\jpeg\jmorecfg.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\webp\include\webp ..\mapnik-sdk\includes\webp /Y
xcopy /i /d /s /q %DEPSDIR%\proj\src\proj_api.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiff.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffvers.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffio.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\cairo-version.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-features.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-deprecated.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-svg.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-svg-surface-private.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-pdf.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-ft.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-ps.h ..\mapnik-sdk\includes\cairo\ /Y
IF ERRORLEVEL 1 GOTO ERROR

:: libs
xcopy /i /d /s /q %DEPSDIR%\freetype\freetype.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\icu\lib\icuuc.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\icu\lib\icuin.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icuuc53.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icudt53.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icuin53.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2_a.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2_a_dll.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff_i.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\zlib-1.2.5\zlib.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\proj\src\proj.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\webp\lib\libwebp.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libpng\projects\vstudio\Release\libpng16.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\libpng\projects\vstudio\Release\libpng16.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\jpeg\libjpeg.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo-static.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\boost_1_55_0\stage\lib\* ..\mapnik-sdk\libs\ /Y
IF ERRORLEVEL 1 GOTO ERROR

:: data
xcopy /i /d /s /q %DEPSDIR%\proj\nad ..\mapnik-sdk\share\proj /Y
::xcopy /i /d /s /q ..\gdal\gdal\data %PREFIX%\share\gdal
IF ERRORLEVEL 1 GOTO ERROR

:: headers for plugins
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\libpq-fe.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\include\postgres_ext.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\include\pg_config_ext.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q %DEPSDIR%\sqlite\sqlite3.h ..\mapnik-sdk\includes\ /Y
::xcopy /i /d /s /q %DEPSDIR%\gdal ..\mapnik-sdk\includes\ /Y

:: libs for plugins
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\Release\libpq.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\Release\libpq.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q %DEPSDIR%\sqlite\sqlite3.lib ..\mapnik-sdk\libs\ /Y

::msbuild /m:2 /t:mapnik /p:BuildInParellel=true .\build\mapnik.sln /p:Configuration=Release

msbuild /m:2 /p:BuildInParellel=true .\build\mapnik.sln /p:Configuration=Release
:: /t:rebuild
:: /v:diag > build.log
IF ERRORLEVEL 1 GOTO ERROR

:: run tests
SET PATH=%CD%\..\mapnik-sdk\libs;%PATH%
for %%t in (build\Release\*test.exe) do ( %%t -d %CD% )

GOTO DONE

:ERROR
echo ----------ERROR MAPNIK --------------

:DONE

EXIT /b %ERRORLEVEL%