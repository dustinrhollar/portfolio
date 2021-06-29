# Low Poly Terrain

## About

This demo procedurally generates terrain and is based on ThinMatrix's series on [Low Poly Terrain](https://www.youtube.com/playlist?list=PLRIWtICgwaX19nkYWDV-U1Ng2C2jnFFLt).

The demo supports the following features:
1. Low Poly Terrain Shader
2. Low Poly Water Shader with reflection/refraction render passes
3. Multiple mesh generation algorithms

![Low Poly Render](https://github.com/dustinrhollar/portfolio/blob/main/LowPolyTerrainGen/showcase/demo.png)

## Compiling

This demo only supports Linux. To compile and run the program:
```
./build.sh # build app and copy shader
./run.sh   # run the app
```

If there is an error running the build script, then it is likely that you have to mark the scrips as executable. 
```
chmod +x scripts/machine.sh
chmod +x build.sh
chmod +x run.sh
```
