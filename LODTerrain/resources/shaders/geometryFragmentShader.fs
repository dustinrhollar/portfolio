#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec4 TexCoord;
in vec3 FragPos;
in vec3 Normal;

//out vec4 FragColor;

uniform sampler2D landscapeTexture;

void main()
{

    gNormal = normalize( (Normal * 2) - 1 );
    //FragColor = vec4( Normal, 1.0 );

    //FragColor = texture(landscapeTexture, TexCoord.ba);
    //FragColor = vec4( gNormal, 1.0f );

    gPosition = FragPos;

    // Specular color
    gAlbedoSpec.rgb = texture(landscapeTexture, TexCoord.ba).rgb;
    // Specular intensity
    gAlbedoSpec.a = 17.0f;


}