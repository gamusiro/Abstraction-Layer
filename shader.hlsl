/*===================================================================================
*	Date : 2022/10/14(Fri)
*		Author	: Gakuto.S
*		File	: vertexShader.hlsl
*		Detail	:
===================================================================================*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float4 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

cbuffer g_worldBuffer : register(b0)
{
    matrix world;
};
cbuffer g_viewBuffer : register(b1)
{
    matrix view;
};
cbuffer g_projectionBuffer : register(b2)
{
    matrix projection;
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT vsmain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPosition    = mul(input.Position, world);
    matrix viewProjection   = mul(view, projection);
    
    /* These params send to pixel shader*/
    output.Position         = mul(worldPosition, viewProjection);
    output.Normal           = input.Normal;
    output.TexCoord         = input.TexCoord;
    
	return output;
}

/*===================================================================================
*	Date : 2022/10/14(Fri)
*		Author	: Gakuto.S
*		File	: pixelShader.hlsl
*		Detail	:
===================================================================================*/
Texture2D<float4>   g_texture : register(t0);
SamplerState        g_sampler : register(s0);

float4 psmain(PS_INPUT input) : SV_TARGET
{
    // float4 texColor = g_texture.Sample(g_sampler, input.TexCoord);
    
    return float4(input.Normal.rgb, 1.0f);
}