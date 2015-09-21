@ECHO OFF
SETLOCAL
SET EL=0

ECHO =========== %~f0 ===========

SET PATH=C:\Python27;%PATH%
SET PATH=C:\Program Files\7-Zip;%PATH%

if EXIST gyp ECHO gyp already cloned && GOTO GYP_ALREADY_HERE
CALL git clone https://chromium.googlesource.com/external/gyp.git gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
:GYP_ALREADY_HERE
CD gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git pull
CD ..
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET DEPS_URL=https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/mapnik-win-sdk-binary-deps-%msvs_toolset%.0-%platform%.7z
ECHO fetching binary deps^: %DEPS_URL%
IF EXIST deps.7z (ECHO already downloaded) ELSE (powershell Invoke-WebRequest "${env:DEPS_URL}" -OutFile deps.7z)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
ECHO extracting binary deps
IF EXIST mapnik-sdk (ECHO already extracted) ELSE (7z -y x deps.7z | %windir%\system32\FIND "ing archive")
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET MAPNIK_SDK=%APPVEYOR_BUILD_FOLDER%\mapnik-sdk

ECHO generating solution file, calling gyp...
CALL gyp\gyp.bat mapnik.gyp --depth=. ^
 --debug=all ^
 -Dincludes=%MAPNIK_SDK%/include ^
 -Dlibs=%MAPNIK_SDK%/lib ^
 -Dconfiguration=%configuration% ^
 -Dplatform=%platform% ^
 -Dboost_version=1_%BOOST_VERSION% ^
 -f msvs -G msvs_version=2015 ^
 --generator-output=build
IF %ERRORLEVEL% NEQ 0 (ECHO error during solution file generation && GOTO ERROR) ELSE (ECHO solution file generated)


GOTO DONE

:ERROR
ECHO =========== ERROR %~f0 ===========
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO =========== DONE %~f0 ===========

EXIT /b %EL%
