# Heightmap CUDA

## About

This app is a small demo demonstrating interopability between C++, CUDA, and OpenGL. Perlin Noise is calculated within a CUDA kernal, and then blitted to an OpenGL PBO. 

[Heightmap Demo](https://github.com/dustinrhollar/portfolio/blob/main/HeightmapCUDA/showcase/heightmap.PNG)

## Compiling

This demo only supports Windows. To compile and run the program:
```
build.bat # builds cuda kernal, c++, and shaders
run.bat   # runs the program
```
