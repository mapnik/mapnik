@ECHO OFF
SETLOCAL
SET EL=0

ECHO =========== %~f0 ===========

ECHO NUMBER_OF_PROCESSORS^: %NUMBER_OF_PROCESSORS%
ECHO RAM [MB]^:
powershell "get-ciminstance -class 'cim_physicalmemory' | %% { $_.Capacity/1024/1024}"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

::only build on AppVeyor, if explicitly stated
ECHO APPVEYOR_REPO_COMMIT_MESSAGE^: %APPVEYOR_REPO_COMMIT_MESSAGE%
::SET BUILD_ON_APPVEYOR=0
::for /F "tokens=1 usebackq" %%i in (`powershell .\scripts\parse-commit-message.ps1 '[build appveyor]'`) DO SET BUILD_ON_APPVEYOR=%%i
::IF %BUILD_ON_APPVEYOR% EQU 0 ECHO not building, commit with [build appveyor] && GOTO DONE

ECHO configuration^: %configuration%
ECHO platform^: %platform%
ECHO msvs_toolset^: %msvs_toolset%
SET BUILD_TYPE=%configuration%
SET BUILDPLATFORM=%platform%
SET TOOLS_VERSION=%msvs_toolset%.0
IF DEFINED APPVEYOR (ECHO on AppVeyor) ELSE (ECHO NOT on AppVeyor)
ECHO ========

SET PATH=C:\Python27;%PATH%
SET PATH=C:\Program Files\7-Zip;%PATH%
:: *nix style find command:
SET PATH=C:\Program Files (x86)\Git\bin;%PATH%

::cloning mapnik-gyp
if EXIST mapnik-gyp ECHO mapnik-gyp already cloned && GOTO MAPNIK_GYP_ALREADY_HERE
CALL git clone https://github.com/mapnik/mapnik-gyp.git
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
:MAPNIK_GYP_ALREADY_HERE
CD mapnik-gyp
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
git pull
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

SET AV_MAPNIK_GYP_STARTTIME=%TIME%
ECHO calling build.bat of mapnik-gyp && CALL build.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
ECHO %AV_MAPNIK_GYP_STARTTIME% started mapnik-gyp build.bat
ECHO %TIME% finished mapnik-gyp build.bat

GOTO DONE

:ERROR
ECHO =========== ERROR %~f0 ===========
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO =========== DONE %~f0 ===========

EXIT /b %EL%
