CALL gyp\gyp.bat mapnik.gyp --depth=. ^
 -Dincludes=%CD%/../mapnik-sdk/includes ^
 -Dlibs=%CD%/../mapnik-sdk/libs ^
 -f msvs -G msvs_version=2013 ^
 --generator-output=build ^
 --no-duplicate-basename-check

mkdir ..\mapnik-sdk
mkdir ..\mapnik-sdk\includes
mkdir ..\mapnik-sdk\share
mkdir ..\mapnik-sdk\libs
mkdir ..\mapnik-sdk\

:: includes
xcopy /i /d /s /q ..\boost_1_55_0\boost ..\mapnik-sdk\includes\boost /Y
xcopy /i /d /s /q ..\icu\include\unicode ..\mapnik-sdk\includes\unicode /Y
xcopy /i /d /s /q ..\freetype\include ..\mapnik-sdk\includes\freetype2 /Y
xcopy /i /d /s /q ..\libxml2\include\libxml ..\mapnik-sdk\includes\libxml /Y
xcopy /i /d /s /q ..\zlib-1.2.5\zlib.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\zlib-1.2.5\zconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libpng\png.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libpng\pnglibconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libpng\pngconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\jpeg\jpeglib.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\jpeg\jconfig.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\jpeg\jmorecfg.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\webp\include\webp ..\mapnik-sdk\includes\webp /Y
xcopy /i /d /s /q ..\proj\src\proj_api.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libtiff\libtiff\tiff.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libtiff\libtiff\tiffvers.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libtiff\libtiff\tiffconf.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\libtiff\libtiff\tiffio.h ..\mapnik-sdk\includes\ /Y
xcopy /i /d /s /q ..\cairo\cairo-version.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-features.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-deprecated.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-svg.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-svg-surface-private.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-pdf.h ..\mapnik-sdk\includes\cairo\ /Y
xcopy /i /d /s /q ..\cairo\src\cairo-ft.h ..\mapnik-sdk\includes\cairo\ /Y

:: libs
xcopy /i /d /s /q ..\zlib-1.2.5\zlib.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\proj\src\proj.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\webp\lib\libwebp.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\libpng\libpng.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\jpeg\libjpeg.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\cairo\src\release\cairo-static.lib ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\cairo\src\release\cairo.dll ..\mapnik-sdk\libs\ /Y
xcopy /i /d /s /q ..\boost_1_55_0\stage\lib\* ..\mapnik-sdk\libs\ /Y

cairo-deprecated.h
cairo-svg.h
cairo-pdf.h
cairo-svg-surface-private.h
:: data
xcopy /i /d /s /q ..\proj\nad ..\mapnik-sdk\share\proj /Y

::xcopy /i /d /s /q ..\gdal\gdal\data %PREFIX%\share\gdal
msbuild /m .\build\mapnik.sln /t:Rebuild /p:Configuration=Release
:: /v:diag > build.log