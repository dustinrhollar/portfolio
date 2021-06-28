#version 330 core

flat in vec3  pass_color;
out vec4 FragColor;

const float near = 0.01;
const float far = 100.0;

void main()
{
    FragColor = vec4(pass_color, 1.0f);
        
    //float depth = gl_FragCoord.z;
    //float linear = linearize_depth(depth) / far;
    //FragColor = vec4(vec3(linear), 1.0f);
}
