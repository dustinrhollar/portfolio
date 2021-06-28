#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 indicators;

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

uniform float height;
uniform float wave_time;

out vec4 clip_space;
out vec4 clip_space_grid; // w/o distortion
out vec3 to_camera;
out vec3 pass_normal;
out vec3 pass_specular;
out vec3 pass_diffuse;

const float PI = 3.1415926535897932384626433832795;
const float wave_length = 4.0;
const float wave_amplitude = 0.2;
const float specular_reflectivity = 0.4;
const float shine_damper = 20.0;

float gen_offset(float x, float z, float val1, float val2)
{
    float radiansX = ((mod(x+z*x*val1, wave_length)/wave_length) + wave_time * mod(x * 0.8 + z, 1.5)) * 2.0 * PI;
	float radiansZ = ((mod(val2 * (z*x +x*z), wave_length)/wave_length) + wave_time * 2.0 * mod(x , 2.0) ) * 2.0 * PI;
	return wave_amplitude * 0.5 * (sin(radiansZ) + cos(radiansX));
}

vec3 distort(vec3 vertex)
{
    float x = gen_offset(vertex.x, vertex.z, 0.2, 0.1);
    float y = gen_offset(vertex.x, vertex.z, 0.1, 0.3);
    float z = gen_offset(vertex.x, vertex.z, 0.15, 0.2);
    return vertex + vec3(x, y, z);
}

vec3 calc_normal(vec3 v0, vec3 v1, vec3 v2)
{
    vec3 tan = v1 - v0;
    vec3 bitan = v2 - v0;
    return normalize(cross(tan, bitan));
}

vec3 calc_specular_lighting(vec3 to_camera, vec3 to_light, vec3 normal)
{
    vec3 reflected_light = reflect(-to_light, normal);
    float spec_factor = dot(reflected_light, to_camera);
    spec_factor = max(spec_factor, 0.0);
    spec_factor = pow(spec_factor, shine_damper);
    return spec_factor * specular_reflectivity * light_color;
}

vec3 calc_diffuse_lighting(vec3 to_light, vec3 normal)
{
    float brightness = max(dot(to_light, normal), 0.0);
    return (light_color * light_bias.x) + (brightness * light_color * light_bias.y);
}

void main()
{
    vec3 vertex = vec3(pos.x, height, pos.y);
    vec3 vertex1 = vertex + vec3(indicators.x, 0.0, indicators.y);
    vec3 vertex2 = vertex + vec3(indicators.z, 0.0, indicators.w);
    
    clip_space_grid = proj * view * vec4(vertex, 1.0);
    
    vec3 distorted_vert = distort(vertex);
    vec3 distorted_vert1 = distort(vertex1);
    vec3 distorted_vert2 = distort(vertex2);
    
    pass_normal = calc_normal(distorted_vert, distorted_vert1, distorted_vert2);

    // usually you would mult. (model * pos), but there is no model matrix
    vec4 world_pos = vec4(distorted_vert, 1.0);
    
    clip_space = proj * view * world_pos;
    gl_Position = clip_space;
    to_camera = normalize(cam_pos - world_pos.xyz);
    
    // lighting
    vec3 to_light = -normalize(light_dir);
    pass_specular = calc_specular_lighting(to_camera, to_light, pass_normal);
    pass_diffuse = calc_diffuse_lighting(to_light, pass_normal);
}
