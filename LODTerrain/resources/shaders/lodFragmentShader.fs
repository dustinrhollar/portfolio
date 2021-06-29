#version 400 core


//in vec4 TexCoord;
//in vec3 FragPos;
//in vec3 Normal;

out vec4 FragColor;

//uniform sampler2D landscapeTexture;

in TES_OUT
{
    vec3 N;
} tes_out;


void main()
{

    //gNormal = normalize( (Normal * 2) - 1 );
    //FragColor = vec4( Normal, 1.0 );

    //FragColor = texture(landscapeTexture, TexCoord.ba);

    // HERE
    //FragColor = vec4( tes_out.N, 1.0f );
	FragColor = vec4( 1.0, 1.0, 1.0, 1.0f );
	
    //gPosition = FragPos;

    // Specular color
    //gAlbedoSpec.rgb = texture(landscapeTexture, TexCoord.ba).rgb;
    // Specular intensity
    //gAlbedoSpec.a = 17.0f;
}