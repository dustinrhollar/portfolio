@echo off

fxc /nologo /Od /Zi /T vs_5_0 /Fosimple_vert.cso simple_vert.hlsl
fxc /nologo /Od /Zi /T ps_5_0 /Fosimple_frag.cso simple_frag.hlsl

