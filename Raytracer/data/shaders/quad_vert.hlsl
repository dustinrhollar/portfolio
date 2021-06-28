struct VS_IN
{
	float3 Position : POSITION;
	float4 Color    : COLOR;
	float2 Tex0     : TEXCOORD;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
	float2 Tex0     : TEXCOORD;
};

VS_OUT main(uint VertId : SV_VERTEXID)
{
    VS_OUT Output;
    
	Output.Tex0     = float2(VertId & 1, VertId>>1);
	Output.Position = float4((Output.Tex0.x - 0.5f) * 2.0f,- (Output.Tex0.y - 0.5f) * 2.0f, 0.0f, 1.0f);
    Output.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Output;
}