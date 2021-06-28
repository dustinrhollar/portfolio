@echo off
setlocal EnableDelayedExpansion

:: Project directory
SET HOST_DIR=%~dp0
SET HOST_DIR=%HOST_DIR:~0,-1%

:: General Flags and whatnot

SET debug_flags=/Od /Z7 /MTd /D_DEBUG
SET release_flags=/O2 /GL /MT /analyze- /D NDEBUG 
SET linker_flags=/INCREMENTAL:no /NOLOGO /SUBSYSTEM:WINDOWS user32.lib shell32.lib Winmm.lib ole32.lib Gdi32.lib D3DCompiler.lib
SET common_flags=/W3 /Gm- /EHsc /nologo /I ..\..\Platform

SET input_main=..\..\UnityBuild.cpp
SET output_main=/Fe:PlatformExample.exe

REM Run the build tools, but only if they aren't set up already.

cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Running VS build tool setup.
echo Initializing MS build tools...
call ..\Common\Scripts\setup_cl.bat
cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Unable to find build tools! Make sure that you have Microsoft Visual Studio 10 or above installed!
exit /b 1

:build

SET mode=debug
IF /i $%1 equ $release (set mode=release)
IF %mode% equ debug (
	SET flags_main=%common_flags% %input_main% %output_main% %debug_flags%
) else (
	SET flags=%common_flags% %release_flags%
)

echo Building in %mode% mode.

IF NOT EXIST bin\%mode% mkdir bin\%mode%

pushd bin\%mode%

echo.     -Compiling C++:
	call cl %flags_main% /link %linker_flags%
	if %errorlevel% neq 0 (
		echo Error during compilation!
		popd
		goto :fail
	)

popd

:complete
echo Build complete!
exit /b 0

:fail
echo Build failed!
exit /b %errorlevel%