// *** GLOBAL VARIABLES ***
float4x4 gWorldViewProj : WorldViewProjection;

// Textures
Texture2D gDiffuseMap : DiffuseMap; // Color texture for our mesh


// SAMPLE OUR SHADER WITH DIFFERENT SAMPLER STATES
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap;
};


//--------------------------------------------------------
//	INPUT / OUTPUT STRUCTS
// Define how a vertex looks like ( vertext layout)
// This need to match with the struct in C++
//--------------------------------------------------------

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};


struct VS_OUTPUT		// All values are interpolated
{
    float4 Position : SV_POSITION; // SV_POSITION is mandatory so the GPU has the needed data for the next drawing step
    float4 WorldPosition : TEXCOORD0;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//--------------------------------------------------------
//	Vertex Shader
//--------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	
    VS_OUTPUT output = (VS_OUTPUT) 0;
	
    float4 pos = float4(input.Position, 1.f);

	// Transform input Position using the WorldViewProjection
    pos = mul(pos, gWorldViewProj);

    output.Position = pos;
    output.Color = input.Color;
    output.TextureUV = input.TextureUV;
    return output;
}


//--------------------------------------------------------
//	Pixel Shader
// SV_TARGET is used to reference which render target you
// want to render
//--------------------------------------------------------
float4 PS_POINT(VS_OUTPUT input) : SV_TARGET
{
	//return float4(input.Color, 1.f);
	return float4(gDiffuseMap.Sample(samPoint, input.TextureUV) * input.Color , 1.f);
   // return CalculatePS(samPoint, input);
	
}


technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_POINT()));
    }
}