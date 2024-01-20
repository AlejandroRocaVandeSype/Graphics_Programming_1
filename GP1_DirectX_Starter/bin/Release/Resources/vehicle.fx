// SHADING MODES
#define SHADING_MODE_OBSERVEDAREA 0
#define SHADING_MODE_DIFFUSE 1
#define SHADING_MODE_SPECULAR 2
#define SHADING_MODE_COMBINED 3

// *** GLOBAL VARIABLES ***
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float3 gCameraPosition : CAMERA;
// Textures
Texture2D gDiffuseMap : DiffuseMap;					// Color texture for our mesh
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
Texture2D gNormalMap : NormalMap;
// Hardcoded values for the lighting
const float3 gLightDirection : LightDirection = float3(.577f, -.577f, .577f);
const float gLightIntensity : LightIntensity = 7.0f;
const float3 gLightAmbient : LightAmbient = float3(0.03f, 0.03f, 0.03f);
const float gShininess : Shininess = 25.0f;

float gPI = 3.141592653;

bool gUseNormalMap : UseNormalMap;
int gShadingMode : ShadingMode;


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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};


struct VS_OUTPUT		// All values are interpolated
{
	float4 Position : SV_POSITION;	// SV_POSITION is mandatory so the GPU has the needed data for the next drawing step
	float4 WorldPosition : TEXCOORD0;
	float3 Color : COLOR;
	float2 TextureUV : TEXCOORD1;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
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
    output.Normal = normalize(mul(normalize(input.Normal), (float3x3) gWorldMatrix)); // Only rotation part is needed -> Convert to 3x3
    output.Tangent = normalize(mul(normalize(input.Tangent), (float3x3) gWorldMatrix));
	output.WorldPosition = mul(input.Position, gWorldMatrix);
	return output;
}




// *** HELPER FUNCTIONS **** 

// LAMBERT'S COSINE LAW -> Measure the OBSERVED AREA
float CalculateObservedArea(SamplerState samplerType, VS_OUTPUT input)
{
    float viewAngle;
    if (gUseNormalMap)
    {
		// NORMAL MAP
		// Create a matrix that makes us able to transform the sampled normal into the correct space
        float3 binormal = cross(input.Normal, input.Tangent);
        float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
	
        float3 sampledNormal = gNormalMap.Sample(samplerType, input.TextureUV);
		// Remap to correct range [-1, 1]
        sampledNormal = 2.f * sampledNormal.rgb - float3(1.f, 1.f, 1.f);
	
        float3 transformedNormal = mul(sampledNormal, tangentSpaceAxis);
	
        viewAngle = saturate(dot(transformedNormal, -gLightDirection));
	
    }
    else
    {
		// Don't use normal map.
        viewAngle = saturate(dot(input.Normal, -gLightDirection));
    }
	
    return viewAngle;
}

/**
	* \param ks Specular Reflection Coefficient
	* \param exp Phong Exponent
	* \param l Incoming (incident) Light Direction
	 * \param v View Direction
	* \param n Normal of the Surface
	* \return Phong Specular Color
*/
float3 Phong(const float4 ks, const float exp, const float3 l, const float3 v, const float3 n)
{
    const float3 reflectVec = reflect(l, n);
	
    float cosAngle = saturate(dot(reflectVec, v)); // Clamp the result between [0, 1]
	
    float specularReflect = ks.r * pow(cosAngle, exp);
	
    return float3(specularReflect, specularReflect, specularReflect);
}

// LAMBERT DIFFUSE REFLECTION
// * Kd -> Diffuse Reflectance 
// * cd -> Diffuse Color [RGB]
float3 Diffuse(float kd, float3 cd)
{
    return (kd * cd) / gPI;
}


// SPECULAR LAMBERT
float3 CalculateSpecular(SamplerState samplerType, VS_OUTPUT input, float viewAngle)
{
    float4 sampledGloss = gGlossinessMap.Sample(samplerType, input.TextureUV);
    float4 sampledSpecular = gSpecularMap.Sample(samplerType, input.TextureUV);
	
	// All parameters have the same value
    sampledGloss.r *= gShininess;
	
	// Calculate the view dir with the interpolated world position of the pixel 
	// and the ONB of the camera
	float3 invViewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);
	
    return Phong(sampledSpecular, sampledGloss.r, gLightDirection, invViewDirection, input.Normal);
	
}

float4 CalculateCombined(SamplerState samplerType, VS_OUTPUT input, float viewAngle)
{		
    float3 specular = CalculateSpecular(samplerType, input, viewAngle);
    float3 diffuse = Diffuse(gLightIntensity, gDiffuseMap.Sample(samplerType, input.TextureUV).rgb);
				
    float3 finalBRDF = diffuse + specular;
			
    float3 finalRGB = (finalBRDF + gLightAmbient.rgb) * viewAngle;
	
    return float4(finalRGB, 1.f);
	
}





//--------------------------------------------------------
//	Helper function for the Pixel Shader
//  Used to sample the texture with the sampler state passed
// through parameter
//--------------------------------------------------------
float4 CalculatePS(SamplerState samplerType, VS_OUTPUT input)
{
    float viewAngle = CalculateObservedArea(samplerType, input);

    switch (gShadingMode)
    {
		case SHADING_MODE_OBSERVEDAREA:
			{             		
                return float4(viewAngle, viewAngle, viewAngle, 1.f);
            }      
		
        case SHADING_MODE_DIFFUSE:
		{
                float3 finalDiffuse = Diffuse(gLightIntensity, gDiffuseMap.Sample(samplerType, input.TextureUV).rgb);
                return float4(finalDiffuse, 1.f);
        }
           	
        case SHADING_MODE_SPECULAR:         
            return float4(CalculateSpecular(samplerType, input, viewAngle) * viewAngle, 1.f);
		
        case SHADING_MODE_COMBINED:
            return CalculateCombined(samplerType, input, viewAngle);
		
		default: // By Default use Combined Shading Mode
            return CalculateCombined(samplerType, input, viewAngle);
          
        
    }
	
}


//--------------------------------------------------------
//	Pixel Shader ( With POINT sampler state filter )
// SV_TARGET is used to reference which render target you
// want to render
//--------------------------------------------------------
float4 PS_POINT(VS_OUTPUT input) : SV_TARGET
{
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
//	Pixel Shader ( With ANISOTROPIC sampler state filter )
//--------------------------------------------------------
float4 PS_ANISOTROPIC(VS_OUTPUT input) : SV_TARGET
{
	return CalculatePS(samAnisotropic, input);
}



RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockWise = false;
};

//--------------------------------------------------------
// Techniques
// Needed because we are using the Effect Framework
// Defines which functions to use for which stage 
//--------------------------------------------------------
technique11 PointTechnique
{
	pass P0
	{
        SetRasterizerState(NULL); 										// Use default rasterizer state
        SetDepthStencilState(NULL, 0);									// Use default depth stencil state
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF); // Use default blend state
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_POINT() ) );
	}
}

technique11 LinearTechnique
{
	pass P0
	{
        SetRasterizerState(NULL); 
        SetDepthStencilState(NULL, 0); 
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF); 
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_LINEAR()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
        SetRasterizerState(NULL); 
        SetDepthStencilState(NULL, 0); 
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ANISOTROPIC()));
	}
}





