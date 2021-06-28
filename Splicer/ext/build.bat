@echo off

IF NOT EXIST build (
    1>NUL md build
)

mkdir build
pushd build
    cmake -G "NMake Makefiles" ..
    nmake
popd

xcopy build\glfw-3.3.2\src\glfw3dll.lib ..\build\ /i /d /y
xcopy build\glfw-3.3.2\src\glfw3.dll    ..\build\ /i /d /y
