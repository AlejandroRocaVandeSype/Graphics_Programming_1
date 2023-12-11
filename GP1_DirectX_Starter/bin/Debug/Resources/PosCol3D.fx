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


struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

// Create our shader functions
//--------------------------------------------------------
//	Vertex Shader
//--------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = float4(input.Position, 1.f);
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