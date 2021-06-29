// C lib
//------------------------------------------
#include <string>
#include <fstream> 
#include <sstream>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <stdint.h>

// External Libs
//------------------------------------------
// GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// STB
#define STB_IMAGE_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_ds.h>

// GLM
// TODO(Dustin): Remove GLM in favor of a lighter math Library
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// GLAD
#include "glad.c"

// IMGUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

// Header files
//------------------------------------------
#include <appconfig.h>
#include <Camera.h>
#include <Shader.h>
#include <Lighting.h>
#include <Renderer.h>
#include <Mesh.h>
#include <OpenSimplexNoise.h>
#include <TerrainGenerator.h>
#include <Terrain.h>
#include <LODMesh.h>
#include <Application.h>

//------------------------------------------
// Source
#include "Lighting.cpp"
#include "Renderer.cpp"
#include "Mesh.cpp"
#include "TerrainGenerator.cpp"
#include "Terrain.cpp"
#include "LODMesh.cpp"
#include "Application.cpp"
#include "LODApp.cpp"
#include "TerrainApp.cpp"
#include "main.cpp"


