struct VS_IN
{
	float3 Position : POSITION;
	float3 Color    : COLOR;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VS_OUT main(VS_IN Input)
{
    VS_OUT Output;

    Output.Position = float4(Input.Position, 1.0f);
    Output.Color    = float4(Input.Color, 1.0f);

    return Output;
}


