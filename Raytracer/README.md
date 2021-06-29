# Maple Raytracer

## About

Maple Raytracer is a demo based on "Ray Tracing in a Weekend" with a twist: the raytracer is multitreaded. The demo uses a job system to submit MxN pixels to be computed by the raytracer. The raytraced image is blitted to a window as it is rendered, allowing for a user to observe the progress of the raytracer.   

The following image is rendered in 3 minutes using 100 samples per pixel and a ray depth of 50.

![Raytracer Demo](https://github.com/dustinrhollar/portfolio/blob/main/Raytracer/showcase/raytracer_demo.PNG)

The demo supports the following features:
- DX11 for image presentation
- Thread Pool for async job execution
- Acceleration Structre
- Sphere primitive
- Lambertian, Dialectric, and Metal materials 

## Compiling

Run the following commands in your preferred command terminal.
```
build.bat        # compiles program
build.bat shad   # compiles and copies shaders
run.bat          # runs the program
```
