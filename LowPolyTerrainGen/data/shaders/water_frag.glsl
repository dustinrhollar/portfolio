#version 330 core

out vec4 FragColor;

in vec4 clip_space;
in vec4 clip_space_grid; // w/o distortion
in vec3 to_camera;
in vec3 pass_normal;
in vec3 pass_specular;
in vec3 pass_diffuse;

uniform sampler2D reflection_tex;
uniform sampler2D refraction_tex;
uniform sampler2D refraction_depth_tex;

const float near = 0.01;
const float far = 1000.0;
const float alpha_depth = 1.0;
const float fresnel_relfective = 0.5;
const float murky_depth = 0.1;
const vec3 water_color = vec3(0.604, 0.867, 0.851);
const float min_blue = 0.4;
const float max_blue = 0.8;

float linearize_depth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near));
}

float fresnel()
{
    vec3 normal = normalize(pass_normal);
    vec3 view = normalize(to_camera);
    float refractive_factor = dot(view, normal);
    return clamp(pow(refractive_factor, fresnel_relfective), 0.0, 1.0); // make the water more/less reflective
}

vec3 apply_murkiness(vec3 refract_color, float water_depth)
{
    //float murky_factor = smoothstep(0, murky_depth, water_depth);
    float murky_factor = clamp(water_depth / murky_depth, 0.0, 1.0);
    float murkiness = min_blue + murky_factor * (max_blue - min_blue);
    return mix(refract_color, water_color, murkiness);
}

vec2 clip_space_to_tex(vec4 clip)
{
    vec2 ndc = clip_space.xy / clip_space.w;
    // clamp to avoid weird wobbly affect at bottom of screen
    return clamp(ndc/2.0 + 0.5, 0.002, 0.998);
}

float get_water_depth(vec2 tex)
{
    float depth = texture(refraction_depth_tex, tex).r;
    float floor_dist = linearize_depth(depth);
    float water_dist = linearize_depth(gl_FragCoord.z);
    return floor_dist - water_dist;
}

void main()
{
    vec2 tex_coords_real = clip_space_to_tex(clip_space);
    vec2 tex_coords_grid = clip_space_to_tex(clip_space_grid);
    
    vec2 refract_tex_coords = tex_coords_grid;
    vec2 reflect_tex_coords = vec2(tex_coords_grid.x, 1 - tex_coords_grid.y);
    float water_depth = get_water_depth(tex_coords_real);
    
    vec3 reflect_color = texture(reflection_tex, reflect_tex_coords).rgb;
    reflect_color = mix(reflect_color, water_color, min_blue);
    
    vec3 refract_color = texture(refraction_tex, refract_tex_coords).rgb;
    refract_color = apply_murkiness(refract_color, water_depth);
    
    vec3 final_color = mix(reflect_color, refract_color, fresnel());
    final_color = final_color * pass_diffuse + pass_specular;
    
    FragColor = vec4(final_color, 1.0);
    FragColor.a = clamp(water_depth/alpha_depth, 0.0, 1.0);
}
