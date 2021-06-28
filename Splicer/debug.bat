@echo off

:debug
pushd build
call devenv splicer.exe
REM call remedybg.exe Sapling.exe
popd