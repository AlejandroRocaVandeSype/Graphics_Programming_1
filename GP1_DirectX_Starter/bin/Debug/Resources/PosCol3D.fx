
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
const float gLightIntensity : LightIntensity = 2.0f;
const float gLightAmbient : LightAmbient = float3(0.03f, 0.03f, 0.03f);
const float gShininess : Shininess = 25.0f;

bool gUseNormalMap : UseNormalMap;
int gShadingMode : ShadingMode;

#define SHADING_MODE_OBSERVEDAREA 0
#define SHADING_MODE_DIFFUSE 1
#define SHADING_MODE_SPECULAR 2
#define SHADING_MODE_COMBINED 3


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
        float4x4 tangentSpaceAxis = float4x4(input.Tangent, 0.0f, binormal, 0.0f, input.Normal, 0.0f, float4(0.0f, 0.0f, 0.0f, 1.0f));
	
        float3 sampledNormal = gNormalMap.Sample(samplerType, input.TextureUV);
		// Remap to correct range [-1, 1]
        sampledNormal = float3(2.f * sampledNormal.r - 1.f, 2.f * sampledNormal.g - 1.f, 2.f * sampledNormal.b - 1.f);
	
        float3 transformedNormal = mul(sampledNormal, tangentSpaceAxis);
	
        viewAngle = dot(transformedNormal, -gLightDirection);
	
    }
    else
    {
		// Don't use normal map.
        viewAngle = dot(input.Normal, -gLightDirection);
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
float4 Phong(const float3 ks, const float exp, const float3 l, const float3 v, const float3 n)
{
    float3 reflect = l - 2 * dot(n, l) * n;
    reflect = normalize(reflect);
	
    float cosAngle = max(0.0f, dot(reflect, v));
	
    float4 specularReflect = float4(ks.r * pow(cosAngle, exp),
									ks.g * pow(cosAngle, exp),
									ks.b * pow(cosAngle, exp), 1.f);
	
    return specularReflect;
}

// LAMBERT PHONG 
float4 CalculateSpecular(SamplerState samplerType, VS_OUTPUT input, float viewAngle, float3 ambient)
{
    float3 sampledGloss = gGlossinessMap.Sample(samplerType, input.TextureUV);
    float3 sampledSpecular = gSpecularMap.Sample(samplerType, input.TextureUV);
	
	// All parameters have the same value
    sampledGloss.r *= gShininess;
	
	float invViewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);
	
    return gLightIntensity * Phong(sampledSpecular, sampledGloss.r, -gLightDirection, invViewDirection, -input.Normal)
	* viewAngle + float4(ambient.r, ambient.g, ambient.b, 1.f);

}

//--------------------------------------------------------
//	Helper function for the Pixel Shader
//  Used to sample the texture with the sampler state passed
// through parameter
//--------------------------------------------------------
float4 CalculatePS(SamplerState samplerType, VS_OUTPUT input)
{
	// Calculate the view dir with the interpolated world position of the pixel 
	// and the ONB of the camera ( For Phong)
    //float invViewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);
	
    float3 ambient = gLightAmbient * input.Color;
    float viewAngle = CalculateObservedArea(samplerType, input);

    switch (gShadingMode)
    {
		case SHADING_MODE_OBSERVEDAREA:
			{
                if (viewAngle < 0.f)
                    return float4(0.f, 0.f, 0.f, 1.f); // If it is below 0 the point on the surface points away from the light
														 // ( It doesn't contribute for the finalColor)			
                return float4(viewAngle, viewAngle, viewAngle, 1.f) + float4(ambient.r, ambient.g, ambient.b, 1.f);
            }      
		
        case SHADING_MODE_DIFFUSE:
            return gLightIntensity * float4(gDiffuseMap.Sample(samplerType, input.TextureUV) * input.Color, 1.f) + float4(ambient.r, ambient.g, ambient.b, 1.f);
		
        case SHADING_MODE_SPECULAR:
            {
                return CalculateSpecular(samplerType, input, viewAngle, ambient);
            }	
		
        case SHADING_MODE_COMBINED:
			{
                if (viewAngle < 0.f)
                    return float4(0.f, 0.f, 0.f, 1.f);
			
                float4 specular = CalculateSpecular(samplerType, input, viewAngle, ambient);
                float4 diffuse = float4(gDiffuseMap.Sample(samplerType, input.TextureUV) * input.Color, 1.f);
				
                float4 finalColor = specular + diffuse;
			
                return gLightIntensity * finalColor * float4(viewAngle, viewAngle, viewAngle, 1.f)
						+ float4(ambient.r, ambient.g, ambient.b, 1.f);
            }
		
		default:
			{
                if (viewAngle < 0.f)
                    return float4(0.f, 0.f, 0.f, 1.f);
			
                return gLightIntensity * float4(gDiffuseMap.Sample(samplerType, input.TextureUV) * input.Color, 1.f) * float4(viewAngle, viewAngle, viewAngle, 1.f)
						+ float4(ambient.r, ambient.g, ambient.b, 1.f);
            }
          
        
    }
	
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
//	Pixel Shader ( With ANISOTROPIC sampler state filter )
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





