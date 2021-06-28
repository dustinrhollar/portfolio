#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec3 col;

layout (std140) uniform GlobalData
{
    mat4 proj;
    mat4 view;
    vec3 cam_pos;
};

layout (std140) uniform LightData
{
    vec3 light_color;
    vec3 light_dir;
    vec2 light_bias;
};

uniform mat4 model;
uniform vec4 clip;

flat out vec3 pass_color;

vec3 calc_lighting()
{
    vec3 normal = norm * 2.0 - 1.0; // remap to -1..1
    float brightness = max(dot(-light_dir, normal), 0.0);
    //return vec3(brightness);
    return (light_color * light_bias.x) + (brightness * light_color * light_bias.y);
}

void main()
{
    vec4 world_pos = model * vec4(pos, 1.0);

    gl_ClipDistance[0] = dot(world_pos, clip);

    gl_Position = proj * view * world_pos;
    
    vec3 lighting = calc_lighting();
    pass_color = col * lighting;    
    //pass_color = col;
}
