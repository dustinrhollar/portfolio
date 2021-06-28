@echo off
setlocal EnableDelayedExpansion

pushd resources\shaders
call build.bat
popd

robocopy resources\shaders build\shaders *.spv /s /purge /ns /nc /ndl /np /njh /njs
robocopy resources\models build\models * /s /purge /ns /nc /ndl /np /njh /njs