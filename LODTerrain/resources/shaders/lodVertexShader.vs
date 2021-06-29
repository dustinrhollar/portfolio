#version 400 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 v_grid_transform;

// View matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Slice information
uniform float vertexSpacing;
uniform vec3 terrainOrigin;
uniform vec3 camera;
uniform vec4 g_transform;

// heightmap information
uniform int width;
uniform int scale;
uniform sampler2D heightmap;

out vec3 vPosition;

void main()
{
    //mat2 grid_transform = mat2( v_grid_transform );
	mat2 grid_transform = mat2( g_transform );
    vec2 t = grid_transform * vec2( vertexSpacing * aPos.x, vertexSpacing * aPos.y);
    vec4 origin = model * vec4( t.x, 0.0, t.y, 1.0 );

    vec2 transform = origin.xz;
	
	float u = (transform.x - terrainOrigin.x) / width;
	float v = (transform.y - terrainOrigin.z) / width;
    vec2 uv = vec2(u, v);
	float y = texture( heightmap, uv ).r; 
	float x = transform.x;
	y *= scale;
	float z = transform.y;
	

    //transform *= vertexSpacing;
	
    //vec3 adjustedVert = vec3( x, y, z );
    vec3 worldPos = vec3( x, y, z ); //model * vec4(adjustedVert, 1.0 );
    // already converted to world coordinates. Might end u being
	vPosition = worldPos; //worldPos.xyz;
}