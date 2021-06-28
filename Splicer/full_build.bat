@echo off
setlocal EnableDelayedExpansion

IF NOT EXIST ..\build\ mkdir ..\build\

REM Build GLFW - only builds GLFW once and copies the dll/lib for
REM subsequent calls
pushd ext
call build.bat
popd

REM Build Shaders and copy any updates resources
call copy_resources.bat

REM Build the application
call build.bat