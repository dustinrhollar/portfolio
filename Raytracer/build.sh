#!/bin/bash

code="$PWD"
opts=-g
cd bin\debug > /dev/null
g++ $opts $code/build.bat -o MapleTerrain.exe
cd $code > /dev/null
