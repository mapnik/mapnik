@ECHO OFF
SETLOCAL
SET EL=0

ECHO =========== %~f0 ===========

SET APPVEYOR_REPO_COMMIT_MESSAGE=this is a [build appveyor] test
SET APPVEYOR=true
::comment this to get complete AppVeyor behaviour
SET LOCAL_BUILD_DONT_SKIP_TESTS=true
SET FASTBUILD=1

:: OVERRIDE PARAMETERS >>>>>>>>
:NEXT-ARG

IF '%1'=='' GOTO ARGS-DONE
ECHO setting %1
SET %1
SHIFT
GOTO NEXT-ARG

:ARGS-DONE
::<<<<< OVERRIDE PARAMETERS


SET configuration=Release
SET msvs_toolset=14
SET platform=x64
SET APPVEYOR_BUILD_FOLDER=%CD%

ECHO pulling test data
CALL git submodule update --init
IF %ERRORLEVEL% NEQ 0 GOTO ERROR
ECHO pulling test data, DONE

SET TIME_START_LOCAL_BUILD=%TIME%
CALL scripts\build-appveyor.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO =========== ERROR %~f0 ===========
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE
ECHO =========== DONE %~f0 ===========
ECHO build started^: %TIME_START_LOCAL_BUILD%
ECHO build finished^: %TIME%

EXIT /b %EL%
