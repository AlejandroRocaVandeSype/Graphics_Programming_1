
// Global variable
float4x4 gWorldViewProj : WorldViewProjection;


cbuffer WorldViewProjConstantBuffer : register(b0)
{
	float4x4 wvpMat;
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
};


struct VS_OUTPUT		// All values are interpolated
{
	float4 Position : SV_POSITION;	// SV_POSITION is mandatory so the GPU has the needed data for the next drawing step
	float3 Color : COLOR;
};

// Create our shader functions
//--------------------------------------------------------
//	Vertex Shader
//--------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 pos = float4(input.Position, 1.f);

	// Transform input Position using the WorldViewProjection
	pos = mul(pos, gWorldViewProj);

	output.Position = pos;
	output.Color = input.Color;
	return output;
}

//--------------------------------------------------------
//	Pixel Shader
// SV_TARGET is used to reference which render target you
// want to render
//--------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return float4(input.Color, 1.f);
}

//--------------------------------------------------------
// Technique
// Needed because we are using the Effect Framework
// Defines which functions to use for which stage 
//--------------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}