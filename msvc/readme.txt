--------------------------------------------------------------------------------
General:
  Built using visual studio c++ 2005 express edition
   - with the platform SDK

 
--------------------------------------------------------------------------------
Boost

http://www.boost.org/users/download/

1) download and extract to "c:/program files/boost/"  or equivalent
    it will make a directory like "c:/program files/boost/boost_1_35_0/"
2) read /index.htm and follow the getting started instructions on compiling the
    libraries for your environment


--------------------------------------------------------------------------------
wxWidgets

http://www.wxwidgets.org/downloads/#latest_stable


1) download and extract the wxAll source archive to /thirdparty/wxWidgets/
    (e.g. there will now be a directory /thirdparty/wxWidgets/wxWidgets-x.x.x/ )
2) open the workspace "wxWidgets/wxWidgets-x.x.x/build/msw/wx.dsw"
    - let visual studio perform conversion if required
3) Select the 'Unicode Debug' configuration and build the complete solution
4) if you want, build the complete solution with the 'Unicode Release' configuration
5) create an environment variable WXDIR with value "path/to/mapnik/thirdparty/wxWidgets/wxWidgets-x.x.x/"


--------------------------------------------------------------------------------
wxPdfDoc

http://wxcode.sourceforge.net/components/wxpdfdoc/

1) download wxPdfDocument 0.8.0 
2) create a directory /thirdparty/wxWidgets/wxCode/components
3) extract wxPdfDocument into /thirdparty/wxWidgets/wxCode/components
4) open the workspace "wxpdfdoc/build/wxpdfdoc.dsw"
    - let visual studio perform conversion if required
5) choose the 'Unicode Debug Monolithic' configuration
6) right click the 'wxpdfdoc' project, Properties and go to C/C++ / General properties
    change WXWIN to WXDIR in the 'Additional include directories' line. It should look like:
    $(WXDIR)\lib\vc_lib\mswud,$(WXDIR)\include,..\include

7) apply the patch wxpdfdoc_patch_20080112.patch or copy the files manually
8) add the files pdfoc.h and pdfoc.cpp to the project

9) build wxpdfdoc
10) repeat step 6 for the 'makefont' project
   It should look like: "$(WXDIR)\lib\vc_lib\mswud";"$(WXDIR)\include";..\include;..\..\include 
   - and also change WXWIN to WXDIR in the resources / General properties 'Additional include directories'
   - and also change WXWIN to WXDIR in the linker / general properties 'additional library directories'
11) In the Linker/input properties, change wxmsw26ud.lib to wxbase28ud.lib and add wxbase28ud_xml.lib
12) build makefont
13) if you want, repeat for the 'Unicode Release Monolithic' configuration
14) create and environment variable WXPDFDIR with value "path/to/mapnik/thirdparty/wxWidgets/wxCode/components/wxpdfdoc"


If this doesnt work, compare the command lines and work out whats different

Example command line for wxpdfdoc project compiler:
  /Od /I "D:\OTM\mapnik\trunk\thirdparty\wxWidgets\wxWidgets-2.8.7\lib\vc_lib\mswud" /I "D:\OTM\mapnik\trunk\thirdparty\wxWidgets\wxWidgets-2.8.7\include" /I "..\include" /D "WIN32" /D "_LIB" /D "_DEBUG" /D "__WXDEBUG__" /D "__WXMSW__" /D "_VC80_UPGRADE=0x0600" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /MDd /Fp".\msvc6prjud_lib\wxpdfdoc/wxpdfdoc_wxpdfdoc.pch" /Fo".\msvc6prjud_lib\wxpdfdoc/" /Fd"..\lib\wxpdfdocud.pdb" /W4 /nologo /c /Zi /TP /errorReport:prompt

Example command line for makefont project compiler:
  /Od /I "D:\OTM\mapnik\trunk\thirdparty\wxWidgets\wxWidgets-2.8.7\lib\vc_lib\mswud" /I "D:\OTM\mapnik\trunk\thirdparty\wxWidgets\wxWidgets-2.8.7\include" /I "..\include" /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "__WXDEBUG__" /D "__WXMSW__" /D "_CONSOLE" /D "_VC80_UPGRADE=0x0600" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /MDd /Fp".\msvc6prjud_lib\makefont/wxpdfdoc_makefont.pch" /Fo".\msvc6prjud_lib\makefont/" /Fd"..\makefont\makefont.pdb" /W4 /nologo /c /Zi /TP /errorReport:prompt

Example command line for makefont project linker:
  /OUT:"..\makefont\makefont.exe" /INCREMENTAL /NOLOGO /LIBPATH:"D:\OTM\mapnik\trunk\thirdparty\wxWidgets\wxWidgets-2.8.7\lib\vc_lib" /LIBPATH:"..\lib" /MANIFEST /MANIFESTFILE:".\msvc6prjud_lib\makefont\makefont.exe.intermediate.manifest" /DEBUG /PDB:".\..\makefont/makefont.pdb" /SUBSYSTEM:CONSOLE /MACHINE:X86 /ERRORREPORT:PROMPT ..\lib\wxpdfdocud.lib wxbase28ud.lib wxbase28ud_xml.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib winmm.lib comctl32.lib rpcrt4.lib wsock32.lib odbc32.lib oleacc.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib



--------------------------------------------------------------------------------
ICU
  
http://icu-project.org/download/
 
1) download and extract to /thirdparty/icu-x.x.x
2) open the solution /icu-x.x.x/source/allinone/allinone.sln
3) Select the 'Debug' configuration and build the complete solution
4) if you want, build the complete solution with the 'Release' configuration 
5) create an environment variable ICUDIR with value "path/to/mapnik/thirdparty/icu-x.x.x/"


--------------------------------------------------------------------------------
freetype

http://freetype.sourceforge.net/download.html#stable
(e.g. http://download.savannah.gnu.org/releases/freetype/freetype-2.3.5.tar.gz)

1) download and extract to /thirdparty/freetype-x.x.x
2) open the solution /freetype-x.x.x/builds/win32/visualc/freetype.sln
3) Select the 'Debug' configuration
4) build the solution
5) if you want, build the solution with the 'Release multithreaded' configuration
6) create an environment variable FREETYPEDIR with value "path/to/mapnik/thirdparty/freetype-x.x.x/"




--------------------------------------------------------------------------------
zlib

http://www.zlib.net/
(http://www.zlib.net/zlib-1.2.3.tar.gz)

1) download and extract to /thirdparty/zlib-x.x.x
2) open the workspace in "/zlib-x.x.x/projects/visualc6/zlib.dsw"
    - let visual studio perform conversion if required
3) select the 'lib debug' configuration and build the zlib project
4) if you want, try the asm optimised versions and release builds
5) create an environment variable ZLIBDIR with value "path/to/mapnik/thirdparty/zlib-x.x.x/"


--------------------------------------------------------------------------------
libpng 

http://www.libpng.org/pub/png/libpng.html
(ftp://ftp.simplesystems.org/pub/libpng/png/src/lpng1225.zip)

1) download and extract to /thirdparty/lpngxxxx
2) open the solution /lpng1225/projects/visualc71/libpng.sln
    - let visual studio perform conversion if required
3) add zlib to the 'additional include directories' of libpng
    e.g. it should look like: ..\..;"..\..\..\zlib-1.2.3"
4) remove the project dependancy on zlib (right click project, project dependancies ...)
5) select the 'lib debug' configuration and build the libpng project
6) if you want, try the asm optimised versions and release builds
2) create an environment variable LIBPNGDIR with value "path/to/mapnik/thirdparty/lpngxxxx/"

--------------------------------------------------------------------------------
libjpeg

http://www.ijg.org/
ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v6b.tar.gz

1) download and extract to /thirdparty/jpeg-6b/
2) copy jconfig.vc to jconfig.h
3) open a visual studio command prompt and change to the /thirdparty/jpeg-x directory
4) nmake /f makefile.vc all

--------------------------------------------------------------------------------
libtiff

http://www.libtiff.org/
(ftp://ftp.remotesensing.org/pub/libtiff/tiff-3.8.2.tar.gz)


1) download and extract to /thirdparty/libtiff-x.x.x
2) open the nmake.opt file and uncomment the JPEG_SUPPORT and ZIP_SUPPORT sections (also update ZLIBDIR and JPEGDIR)
2) open a visual studio command prompt and change to the /thirdparty/libtiff-x.x.x directory
3) nmake /f makefile.vc
4) change to tools directory
5) nmake /f makefile.vc

--------------------------------------------------------------------------------
libtool (ltdl)

http://gnuwin32.sourceforge.net/packages/libtool.htm
(http://gnuwin32.sourceforge.net/downlinks/libtool-bin-zip.php)

1) download and extract the binaries package to /thirdparty/libtool-x.x.x
2) create an environment variable LIBTOOLDIR with value "path/to/mapnik/thirdparty/libtool-x.x.x/"


--------------------------------------------------------------------------------
proj.4

http://www.remotesensing.org/proj/
(ftp://ftp.remotesensing.org/proj/proj-4.6.0.tar.gz)
(ftp://ftp.remotesensing.org/proj/proj-datumgrid-1.3.zip)


1) download and extract to /thirdparty/proj-x.x.x
2) if you have more datum definition files, extract them to the /proj-x.x.x/nad directory
3) edit /proj-x.x.x/src/makefile.vc
    - change PROJ_LIB_DIR to point to the proper place e.g:  PROJ_LIB_DIR=D:\OTM\mapnik\trunk\thirdparty\proj-4.6.0\nad
4) open a visual studio command prompt and change the the /proj-x.x.x/src/ directory
5) nmake /f makefile.vc all
6) create an environment variable PROJ4DIR with value "path/to/mapnik/thirdparty/proj-x.x.x/"


--------------------------------------------------------------------------------
python bindings

1) create an environment variable PYTHONDIR with the value "path/to/python" e.g. "c:\program files\python25"
2) copy "/mapnik/trunk/bindings/python/mapnik" directory to "/python/Lib/site-packages/mapnik"
3) create a file called paths.py in "/python/Lib/site-packages/mapnik/"
    - add the lines:

mapniklibpath = 'path/to/mapnik/msvc/Debug'
inputpluginspath = 'path/to/mapnik/msvc/Debug'
fontscollectionpath = 'path/to/mapnik/fonts/dejavu-ttf-2.14'

e.g.

mapniklibpath = 'D:/otm/mapnik/trunk/msvc/Debug'
inputpluginspath = 'D:/otm/mapnik/trunk/msvc/Debug'
fontscollectionpath = 'D:/otm/mapnik/trunk/fonts/dejavu-ttf-2.14'

4) open __init__.py
    - comment out lines 21-29 and 99, where it is using the get/set dlopenflags things  
5) copy libltdl3.dll from "mapnik/thirdparth/libtool-x.x.x/bin" to "/python/Lib/site-packages/mapnik/"
6) copy boost_python-vc80-mt-gd-x_xx_x.dll from "boost/stage/lib" to "/python/Lib/site-packages/mapnik/"

7) when you build mapnik and the python-bindings they will copy other files to "/python/Lib/site-packages/mapnik/"

-- after you've build mapnik and the python-bindings (later)

8) start python
>>> import mapnik
>>> dir(mapnik)
and you shouldn't get any errors



--------------------------------------------------------------------------------
mapnik

1) create an environment variable MAPNIKDIR with value "path/to/mapnik" e.g. "D:\OTM\mapnik\trunk\"
2) In visual studio, do Tools menu, Options, projects and solutions, vc++ directories
   - choose include files
   - add:
          $(LIBTIFFDIR)\libtiff\
          $(ZLIBDIR)
          $(PROJ4DIR)\src
          $(LIBJPEGDIR)
          $(LIBPNGDIR)
          $(LIBTOOLDIR)\include
          $(FREETYPEDIR)\include
          $(ICUDIR)\include\



--------------------------------------------------------------------------------
building

1) just hit build all and it should all work.
2) to test things, try the pdf project.
  - You'll need to update the four defines at the top of main.cpp in the pdf project.
     Point them to the proper locations on your machine.





--------------------------------------------------------------------------------
alternative libpng

http://gnuwin32.sourceforge.net/packages/libpng.htm
(http://gnuwin32.sourceforge.net/downlinks/libpng-lib-zip.php)

1) download and extract the 'developer files' package to /thirdparty/libpng-x.x.x
2) create an environment variable LIBPNGDIR with value "path/to/mapnik/thirdparty/libpng-x.x.x/"


--------------------------------------------------------------------------------
alternative libtiff

http://gnuwin32.sourceforge.net/packages/tiff.htm
(http://gnuwin32.sourceforge.net/downlinks/tiff-lib-zip.php)

1) download and extract the 'developer files' package to /thirdparty/libtiff-x.x.x-x
2) create an environment variable LIBTIFFDIR with value "path/to/mapnik/thirdparty/libtiff-x.x.x-x/"


--------------------------------------------------------------------------------
alternative libjpeg

http://gnuwin32.sourceforge.net/packages/libjpeg.htm
(http://gnuwin32.sourceforge.net/downlinks/jpeg-lib-zip.php)

1) download and extract the 'developer files' package to /thirdparty/libjpeg-6b
2) create an environment variable LIBJPEGDIR with value "path/to/mapnik/thirdparty/libjpeg-6b/"





