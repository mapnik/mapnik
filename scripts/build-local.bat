@ECHO OFF
SETLOCAL
SET EL=0

ECHO =========== %~f0 ===========

SET APPVEYOR_REPO_COMMIT_MESSAGE=this is a [build appveyor] test
SET APPVEYOR=true
::comment this to get complete AppVeyor behaviour
SET LOCAL_BUILD_DONT_SKIP_TESTS=true

SET MAPNIK_GIT=3.0.5
SET BOOST_VERSION=58
SET FASTBUILD=1
SET configuration=Release
SET msvs_toolset=14
SET platform=x64
SET APPVEYOR_BUILD_FOLDER=%CD%
CALL scripts\build-appveyor.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO =========== ERROR %~f0 ===========
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO =========== DONE %~f0 ===========

EXIT /b %EL%
