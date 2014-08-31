@echo off

::git clone https://chromium.googlesource.com/external/gyp.git
::CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
::SET PATH=C:\Python27;%PATH%

::ddt ..\mapnik-sdk
::IF ERRORLEVEL NEQ 0 GOTO ERROR
::ddt build\Release
::IF ERRORLEVEL NEQ 0 GOTO ERROR

if NOT EXIST gyp (
    CALL git clone https://chromium.googlesource.com/external/gyp.git gyp
    IF %ERRORLEVEL% NEQ 0 GOTO ERROR
)

:: run find command and bail on error
:: this ensures we have the unix find command on path
:: before trying to run gyp
find deps/clipper/src/ -name "*.cpp"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CALL gyp\gyp.bat mapnik.gyp --depth=. ^
 -Dincludes=%CD%/../mapnik-sdk/includes ^
 -Dlibs=%CD%/../mapnik-sdk/libs ^
 -f msvs -G msvs_version=2013 ^
 --generator-output=build ^
 --no-duplicate-basename-check
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

if NOT EXIST ..\mapnik-sdk (
  mkdir ..\mapnik-sdk
  mkdir ..\mapnik-sdk\bin
  mkdir ..\mapnik-sdk\includes
  mkdir ..\mapnik-sdk\share
  mkdir ..\mapnik-sdk\libs
  mkdir ..\mapnik-sdk\libs\mapnik\input
  mkdir ..\mapnik-sdk\libs\mapnik\fonts
)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET DEPSDIR=..

:: includes
xcopy /i /d /s /q %DEPSDIR%\harfbuzz-build\harfbuzz\hb-version.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
::xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-icu.h ..\mapnik-sdk\includes\harfbuzz\ /Y
::IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-shape-plan.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-shape.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-set.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-ft.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-buffer.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-unicode.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-common.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-blob.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-font.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-face.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\harfbuzz\src\hb-deprecated.h ..\mapnik-sdk\includes\harfbuzz\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\boost_1_56_0\boost ..\mapnik-sdk\includes\boost /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\include\unicode ..\mapnik-sdk\includes\unicode /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\freetype\include ..\mapnik-sdk\includes\freetype2 /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libxml2\include ..\mapnik-sdk\includes\libxml2 /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\zlib\zlib.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\zlib\zconf.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libpng\png.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libpng\pnglibconf.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libpng\pngconf.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\jpeg\jpeglib.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\jpeg\jconfig.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\jpeg\jmorecfg.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\webp\src\webp ..\mapnik-sdk\includes\webp /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\proj\src\proj_api.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiff.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffvers.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffconf.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\tiffio.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\cairo-version.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-features.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-deprecated.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-svg.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-svg-surface-private.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-pdf.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-ft.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\cairo-ps.h ..\mapnik-sdk\includes\cairo\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\protobuf\vsprojects\include ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: libs
xcopy /i /d /s /q %DEPSDIR%\harfbuzz-build\harfbuzz.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\freetype\freetype.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\lib\icuuc.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\lib\icuin.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icuuc53.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icudt53.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\icu\bin\icuin53.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2_a.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2_a_dll.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libxml2\win32\bin.msvc\libxml2.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
::xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff.lib ..\mapnik-sdk\libs\ /Y
::IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libtiff\libtiff\libtiff_i.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\zlib\zlib.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\proj\src\proj.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\webp\output\release-dynamic\x86\lib\libwebp_dll.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\webp\output\release-dynamic\x86\bin\libwebp.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libpng\projects\vstudio\Release\libpng16.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\libpng\projects\vstudio\Release\libpng16.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\jpeg\libjpeg.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo-static.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\cairo\src\release\cairo.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\boost_1_56_0\stage\lib\* ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\protobuf\vsprojects\Release\libprotobuf-lite.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: data
xcopy /i /d /s /q %DEPSDIR%\proj\nad ..\mapnik-sdk\share\proj /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\data ..\mapnik-sdk\share\gdal
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: bin
xcopy /i /d /s /q %DEPSDIR%\protobuf\vsprojects\Release\protoc.exe ..\mapnik-sdk\bin /Y
xcopy /i /d /s /q mapnik-config.bat ..\mapnik-sdk\bin /Y

:: headers for plugins
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\libpq-fe.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\include\postgres_ext.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\include\pg_config_ext.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\sqlite\sqlite3.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
::xcopy /i /d /s /q %DEPSDIR%\gdal\gcore\*h ..\mapnik-sdk\includes\gdal\ /Y
::IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_feature.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_spatialref.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_geometry.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_core.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_featurestyle.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogrsf_frmts\ogrsf_frmts.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\ogr\ogr_srs_api.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gcore\gdal_priv.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gcore\gdal_frmts.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gcore\gdal.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gcore\gdal_version.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_minixml.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_atomic_ops.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_string.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_conv.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_vsi.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_virtualmem.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_error.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_progress.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_port.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\port\cpl_config.h ..\mapnik-sdk\includes\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: libs for plugins
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\Release\libpq.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\postgresql\src\interfaces\libpq\Release\libpq.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\sqlite\sqlite3.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gdal_i.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
:: NOTE: impossible to statically link gdal due to:
:: http://stackoverflow.com/questions/4596212/c-odbc-refuses-to-statically-link-to-libcmt-lib-under-vs2010
::xcopy /i /d /s /q %DEPSDIR%\gdal\gdal.lib ..\mapnik-sdk\libs\ /Y
::IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\gdal\gdal111.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\expat\win32\bin\Release\libexpat.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q %DEPSDIR%\expat\win32\bin\Release\libexpat.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: detect trouble with mimatched linking
dumpbin /directives ..\mapnik-sdk\libs\*lib | grep LIBCMT

::msbuild /m:2 /t:mapnik /p:BuildInParellel=true .\build\mapnik.sln /p:Configuration=Release

msbuild /m:2 /p:BuildInParellel=true .\build\mapnik.sln /p:Configuration=Release
:: /t:rebuild
:: /v:diag > build.log
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: install mapnik libs
xcopy /i /d /s /q .\build\Release\mapnik.lib ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q .\build\Release\mapnik.dll ..\mapnik-sdk\libs\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: plugins
xcopy /i /d /s /q .\build\Release\*input ..\mapnik-sdk\libs\mapnik\input\ /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: install mapnik headers
xcopy /i /d /s /q .\deps\mapnik\sparsehash ..\mapnik-sdk\includes\mapnik\sparsehash /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q .\deps\agg\include ..\mapnik-sdk\includes\mapnik\agg /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q .\deps\clipper\include ..\mapnik-sdk\includes\mapnik\agg /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
xcopy /i /d /s /q .\include\mapnik ..\mapnik-sdk\includes\mapnik /Y
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:: run tests
SET PATH=%CD%\..\mapnik-sdk\libs;%PATH%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
for %%t in (build\Release\*test.exe) do ( call %%t -d %CD% )
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
echo ----------ERROR MAPNIK --------------
echo ERRORLEVEL %ERRORLEVEL%

:DONE
echo DONE building Mapnik

EXIT /b %ERRORLEVEL%
