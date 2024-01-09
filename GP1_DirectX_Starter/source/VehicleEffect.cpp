#include "pch.h"
#include "VehicleEffect.h"
#include "Texture.h"
using namespace dae;

VehicleEffect::VehicleEffect(ID3D11Device* pDevice, const std::wstring& assetPath)
	: Effect(pDevice, assetPath),
	m_pPointTech{ nullptr }, m_pLinearTech{ nullptr }, m_pAnisotropicTech{ nullptr },
	m_pDiffuseMapVar{ nullptr }, m_pSpecularMapVar{ nullptr }, m_pGlossinessMapVar{ nullptr }, m_pNormalMapVar{ nullptr },
	m_pWorldVar{ nullptr }, m_pCameraPosVar{ nullptr },
	m_pUseNormalMapVar{ nullptr }, m_pShadingModeVar{ nullptr },
	m_CurrentTech{ 0 }, m_NrTechniques{ 3 }
{
	if (m_pEffect)
	{
		// Bind with our shader ( Capture necessary variables from the GPU)
		ShaderBinding();
	}
}

// Bind everything needed from the shader with our effect
void VehicleEffect::ShaderBinding()
{
	// Techniques
	m_pPointTech = Effect::GetTechnique("PointTechnique");
	m_pLinearTech = Effect::GetTechnique("LinearTechnique");
	m_pAnisotropicTech = Effect::GetTechnique("AnisotropicTechnique");

	// ALWAYS START WITH POINT FILTER 
	std::cout << "SAMPLER_STATE = POINT" << std::endl;
	m_pTechnique = m_pPointTech;

	// Matrices
	m_pWorldVar = GetMatrix("gWorldMatrix");

	// Shader Resources
	m_pDiffuseMapVar = GetShaderResource("gDiffuseMap");
	m_pSpecularMapVar = GetShaderResource("gSpecularMap");
	m_pGlossinessMapVar = GetShaderResource("gGlossinessMap");
	m_pNormalMapVar = GetShaderResource("gNormalMap");

	// Rendering parameters
	m_pUseNormalMapVar = m_pEffect->GetVariableByName("gUseNormalMap")->AsScalar();
	if (!m_pUseNormalMapVar->IsValid())
	{
		std::cout << "gUseNormalMap variable is not valid\n";
	}
	m_pUseNormalMapVar->SetBool(true);
	m_pShadingModeVar = m_pEffect->GetVariableByName("gShadingMode")->AsScalar();
	if (!m_pUseNormalMapVar->IsValid())
	{
		std::cout << "gShadingMode variable is not valid\n";
	}
	m_pShadingModeVar->SetInt(3);  // Combined at start

	// Vector variables
	m_pCameraPosVar = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();
	if (!m_pCameraPosVar->IsValid())
	{
		std::cout << "gCameraPosition variable is not valid\n";
	}
}

void VehicleEffect::UpdateWorldMatrix(const Matrix& worldMatrix)
{
	m_pWorldVar->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void VehicleEffect::ToggleTechnique()
{
	m_CurrentTech++;
	if (m_CurrentTech >= m_NrTechniques)
		m_CurrentTech = 0;

	switch (m_CurrentTech)
	{
		case 0:		// POINT TECHNIQUE
		{
			std::cout << "SAMPLER_STATE = POINT" << std::endl;
			m_pTechnique = m_pPointTech;
			break;
		}
		
		case 1:		// LINEAR TECHNIQUE
		{
			std::cout << "SAMPLER_STATE = LINEAR" << std::endl;
			m_pTechnique = m_pLinearTech;
			break;
		}
		case 2:		// ANISOTROPIC TECHNIQUE
		{
			std::cout << "SAMPLER_STATE = ANISOTROPIC" << std::endl;
			m_pTechnique = m_pAnisotropicTech;
			break;
		}
		
	}
}

void VehicleEffect::ToggleNormalMap()
{
	if (m_pUseNormalMapVar)
	{
		bool useNormalMap{ true };
		m_pUseNormalMapVar->GetBool(&useNormalMap);

		useNormalMap = !useNormalMap;
		m_pUseNormalMapVar->SetBool(useNormalMap);

		if (useNormalMap)
		{
			std::cout << "Sample Normal Map : ON\n";
		}
		else
		{
			std::cout << "Sample Normal Map : OFF\n";
		}
	}
}

void VehicleEffect::ToggleShadingMode(const int shadingMode)
{
	m_pShadingModeVar->SetInt(shadingMode);
}

void VehicleEffect::SetDiffuseMap(const Texture* pDiffuseText)
{
	if (m_pDiffuseMapVar)
		m_pDiffuseMapVar->SetResource(pDiffuseText->GetRSV());

	delete pDiffuseText;
}

void VehicleEffect::SetSpecularMap(const Texture* pSpecularText)
{
	if (m_pSpecularMapVar)
		m_pSpecularMapVar->SetResource(pSpecularText->GetRSV());
}
void VehicleEffect::SetGlossinessMap(const Texture* pGlossinessText)
{
	if (m_pGlossinessMapVar)
		m_pGlossinessMapVar->SetResource(pGlossinessText->GetRSV());
}

void VehicleEffect::SetNormalMap(const Texture* pNormalText)
{
	if (m_pNormalMapVar)
		m_pNormalMapVar->SetResource(pNormalText->GetRSV());
}

void VehicleEffect::SetMaps(const std::unordered_map<std::string, Texture*>& textures)
{

	SetShaderResource(textures, m_pDiffuseMapVar, "diffuse");
	SetShaderResource(textures, m_pGlossinessMapVar, "glossiness");
	SetShaderResource(textures, m_pSpecularMapVar, "specular");
	SetShaderResource(textures, m_pNormalMapVar, "normal");

}

void VehicleEffect::SetShaderResource(const std::unordered_map<std::string, Texture*>& textures, 
	ID3DX11EffectShaderResourceVariable* resourceMap, const std::string& key)
{
	auto textureItr = textures.find(key);
	if (textureItr != textures.end())
	{
		resourceMap->SetResource(textureItr->second->GetRSV());
	}
	else
	{
		std::cout << "Error! Couldn't find " + key + " texture. \n";
	}
}

void VehicleEffect::SetCameraVar(const Vector3& cameraPos)
{
	if (m_pCameraPosVar)
		m_pCameraPosVar->SetFloatVector(reinterpret_cast<const float*>(&cameraPos));
}

// Return the current technique being used
//ID3DX11EffectTechnique* VehicleEffect::GetTechnique() const
//{
//	switch (m_CurrentTech)
//	{
//	case 0:		// POINT TECHNIQUE
//		return m_pPointTech;
//	case 1:		// LINEAR TECHNIQUE
//		return m_pLinearTech;
//	case 2:		// ANISOTROPIC TECHNIQUE
//		return m_pAnisotropicTech;
//	default:
//		return m_pPointTech;
//	}
//}

VehicleEffect::~VehicleEffect()
{
	if (m_pCameraPosVar)
	{
		m_pCameraPosVar->Release();
		m_pCameraPosVar = nullptr;
	}

	if (m_pUseNormalMapVar)
	{
		m_pUseNormalMapVar->Release();
		m_pUseNormalMapVar = nullptr;
	}

	if (m_pNormalMapVar)
	{
		m_pNormalMapVar->Release();
		m_pNormalMapVar = nullptr;
	}
	if (m_pGlossinessMapVar)
	{
		m_pGlossinessMapVar->Release();
		m_pGlossinessMapVar = nullptr;
	}
	if (m_pSpecularMapVar)
	{
		m_pSpecularMapVar->Release();
		m_pSpecularMapVar = nullptr;
	}
	if (m_pDiffuseMapVar)
	{
		m_pDiffuseMapVar->Release();
		m_pDiffuseMapVar = nullptr;
	}

	if (m_pWorldVar)
	{
		m_pWorldVar->Release();
		m_pWorldVar = nullptr;
	}

	if (m_pPointTech)
	{
		m_pPointTech->Release();
		m_pPointTech = nullptr;
	}

	if (m_pLinearTech)
	{
		m_pLinearTech->Release();
		m_pLinearTech = nullptr;
	}

	if (m_pAnisotropicTech)
	{
		m_pAnisotropicTech->Release();
		m_pAnisotropicTech = nullptr;
	}
}