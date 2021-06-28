@echo off
setlocal EnableDelayedExpansion

:: Project directory
SET HOST_DIR=%~dp0
SET HOST_DIR=%HOST_DIR:~0,-1%

:: Cuda Includes
SET CUDA_INC=/I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.2\include"
SET CUDA_LIB=/LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.2\lib\x64" cudart.lib

:: General Flags and whatnot

SET debug_flags=/Od /Z7 /MTd
SET release_flags=/O2 /GL /MT /analyze- /D NDEBUG 
SET linker_flags=/INCREMENTAL:no /NOLOGO /SUBSYSTEM:WINDOWS user32.lib shell32.lib Winmm.lib ole32.lib opengl32.lib Gdi32.lib %CUDA_LIB%
SET common_flags=/Zc:strictStrings- /W3 /Gm- /EHsc /nologo /I ..\..\src /I ..\..\ext %CUDA_INC%

SET input_main=..\..\src\UnityBuild.cpp
SET output_main=/Fe:MapleHeightmap.exe

REM Run the build tools, but only if they aren't set up already.

cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Running VS build tool setup.
echo Initializing MS build tools...
call scripts\setup_cl.bat
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

echo.     -Compiling CUDA Source:
	SET HOST_COMPILER=cl
	SET NVCC= nvcc -ccbin %HOST_COMPILER% -cudart static -DWIN32
	SET NVCC_DEBUG=-Xcompiler "/Od /Z7 /MTd /I..\..\src"
	SET GENCODE_FLAGS=-arch=compute_60 -code=sm_60
	SET NVCC_FLAGS=%NVCC_DEBUG% -c -m64
	SET CUDA_INPUT=..\..\src\Cuda\CudaUnity.cu

	call %NVCC% %NVCC_FLAGS% %GENCODE_FLAGS% %CUDA_INPUT%

echo.     -Compiling C++:
	call cl %flags_main% CudaUnity.obj /link %linker_flags%
	if %errorlevel% neq 0 (
		echo Error during compilation!
		popd
		goto :fail
	)

popd

REM Copy shaders, if they have changed.
if exist data\shaders (
	echo.     -Copying Shaders:
	if not exist bin\%mode%\shaders mkdir bin\%mode%\shaders
	robocopy data\shaders bin\%mode%\shaders *.vert *.frag /s /purge /ns /nc /ndl /np /njh /njs
)

echo Build complete!
exit /b 0

:fail
echo Build failed!
exit /b %errorlevel%