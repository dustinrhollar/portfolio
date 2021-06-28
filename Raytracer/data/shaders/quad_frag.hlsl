
struct PS_IN
{
	float4 Position : SV_POSITION; // interpolated vertex value
	float4 Color    : COLOR;       // interpolated diffuse color
	float2 Tex0     : TEXCOORD;
};

Texture2D<float3> DiffuseTexture : register(t0);
SamplerState SampleType;

float4 main(PS_IN Input) : SV_TARGET
{
	return float4(DiffuseTexture.Sample(SampleType, Input.Tex0), 1.0f);
}