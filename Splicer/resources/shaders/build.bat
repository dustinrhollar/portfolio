@echo off

for %%i in (*.vert *.frag) do "C:\VulkanSDK\1.2.170.0\Bin\glslc.exe" "%%~i" -o "%%~i.spv"