@ECHO OFF
SETLOCAL
SET EL=0

ECHO =========== %~f0 ===========

SET PATH=C:\Python27;%PATH%
SET PATH=C:\Program Files\7-Zip;%PATH%
:: *nix style find command:
SET PATH=C:\Program Files (x86)\Git\bin\find.exe;%PATH%

::cloning mapnik-gyp
if EXIST mapnik-gyp ECHO mapnik-gyp already cloned && GOTO MAPNIK_GYP_ALREADY_HERE
CALL git clone https://github.com/mapnik/mapnik-gyp.git
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
:MAPNIK_GYP_ALREADY_HERE
CD mapnik-gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git pull
IF %ERRORLEVEL% NEQ 0 GOTO ERROR


::cloning gyp
if EXIST gyp ECHO gyp already cloned && GOTO GYP_ALREADY_HERE
CALL git clone https://chromium.googlesource.com/external/gyp.git gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
:GYP_ALREADY_HERE
CD gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git pull
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
CD ..
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET DEPS_URL=https://mapbox.s3.amazonaws.com/windows-builds/windows-build-deps/mapnik-win-sdk-binary-deps-%msvs_toolset%.0-%platform%.7z
ECHO fetching binary deps^: %DEPS_URL%
IF EXIST deps.7z (ECHO already downloaded) ELSE (powershell Invoke-WebRequest "${env:DEPS_URL}" -OutFile deps.7z)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
ECHO extracting binary deps
IF EXIST mapnik-sdk (ECHO already extracted) ELSE (7z -y x deps.7z | %windir%\system32\FIND "ing archive")
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO calling build.bat of mapnik-gyp && CALL build.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO =========== ERROR %~f0 ===========
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO =========== DONE %~f0 ===========

EXIT /b %EL%
