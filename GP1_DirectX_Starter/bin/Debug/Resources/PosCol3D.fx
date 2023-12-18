
// Global variables
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;					// Color texture for our mesh


// SAMPLE OUR SHADER WITH DIFFERENT SAMPLER STATES
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;		// or Mirror, Clamp, Border
	AddressV = Wrap;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;		
	AddressV = Wrap;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;		
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
};


struct VS_OUTPUT		// All values are interpolated
{
	float4 Position : SV_POSITION;	// SV_POSITION is mandatory so the GPU has the needed data for the next drawing step
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD;
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
	output.TextureUV = input.TextureUV;
	return output;
}

//--------------------------------------------------------
//	Helper function for the Pixel Shader
//  Used to sample the texture with the sampler state passed
// through parameter
//--------------------------------------------------------
float4 CalculatePS(SamplerState samplerType, VS_OUTPUT input)
{
	return float4(gDiffuseMap.Sample(samplerType, input.TextureUV) * input.Color, 1.f);
}


//--------------------------------------------------------
//	Pixel Shader ( With POINT sampler state filter )
// SV_TARGET is used to reference which render target you
// want to render
//--------------------------------------------------------
float4 PS_POINT(VS_OUTPUT input) : SV_TARGET
{
	//return float4(input.Color, 1.f);
	//return float4(gDiffuseMap.Sample(samPoint, input.TextureUV) * input.Color , 1.f);
	return CalculatePS(samPoint, input);
}

//--------------------------------------------------------
//	Pixel Shader ( With LINEAR sampler state filter )
//--------------------------------------------------------
float4 PS_LINEAR(VS_OUTPUT input) : SV_TARGET
{
	return CalculatePS(samLinear, input);
}

//--------------------------------------------------------
//	Pixel Shader ( With LINEAR sampler state filter )
//--------------------------------------------------------
float4 PS_ANISOTROPIC(VS_OUTPUT input) : SV_TARGET
{
	return CalculatePS(samAnisotropic, input);
}

//--------------------------------------------------------
// Techniques
// Needed because we are using the Effect Framework
// Defines which functions to use for which stage 
//--------------------------------------------------------
technique11 PointTechnique
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_POINT() ) );
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_LINEAR()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ANISOTROPIC()));
	}
}


