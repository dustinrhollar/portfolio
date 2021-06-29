@echo off
setlocal EnableDelayedExpansion

SET EXE_NAME=ECS.exe

:: Build Commands
SET NONE=""
SET CONFIGURE=conf
SET BUILD=build
SET CLEAN=clean
SET DEBUG=debug

GOTO :MAIN
EXIT /B %ERRORLEVEL%

:Configure
    pushd build
        cmake -G "NMake Makefiles" ..
    popd
    EXIT /B 0

:Build
    pushd build
		nmake
    popd
    EXIT /B 0

:Clean
    del /s /q /f build\*
    EXIT /B 0

:Debug
    pushd build\examples
		devenv .\%EXE_NAME%
    popd
    EXIT /B 0

:MAIN
    IF "%1" == %NONE% (
        pushd build\examples
            .\%EXE_NAME% > log.txt
        popd
		EXIT /B %ERRORLEVEL%
    )

    IF %1 == %CONFIGURE% (
        CALL :Configure
		EXIT /B %ERRORLEVEL%
    )
    IF %1 == %BUILD% (
        CALL :Build
		EXIT /B %ERRORLEVEL%
    )
    IF %1 == %CLEAN% (
        CALL :Clean
		EXIT /B %ERRORLEVEL%
    )
	IF %1 == %DEBUG% (
        CALL :Debug
		EXIT /B %ERRORLEVEL%
    )