// *** GLOBAL VARIABLES ***
float4x4 gWorldViewProj : WorldViewProjection;

// Textures
Texture2D gDiffuseMap : DiffuseMap; // Color texture for our mesh


const float gLightIntensity : LightIntensity = 7.0f;
float gPI = 3.141592653;

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

// LAMBERT DIFFUSE REFLECTION
// * Kd -> Diffuse Reflectance 
// * cd -> Diffuse Color [RGB]
float4 Diffuse(float kd, float4 cd)
{
    return (kd * cd) / gPI;
}

//--------------------------------------------------------
//	Pixel Shader
// SV_TARGET is used to reference which render target you
// want to render
//--------------------------------------------------------
float4 PS_POINT(VS_OUTPUT input) : SV_TARGET
{    
    // No lighting calculation. Only diffuseMap color
    return Diffuse(gLightIntensity, gDiffuseMap.Sample(samPoint, input.TextureUV));
	
}

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockWise = false; 
};

// BLEND returned value
BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

// Perform a depth test without writing into the depth buffer
DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
    
    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;

};


technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_POINT()));
       
    }
}