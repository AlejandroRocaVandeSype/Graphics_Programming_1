#include "pch.h"
#include "Effect.h"
#include "Texture.h"

using namespace dae;


Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetPath)
	: m_pEffect{ nullptr },
	m_pPointTech{ nullptr }, m_pLinearTech{ nullptr }, m_pAnisotropicTech{ nullptr }, 
	m_pDiffuseMapVar {nullptr }, m_pSpecularMapVar{ nullptr }, m_pGlossinessMapVar{ nullptr }, m_pNormalMapVar{ nullptr },
	m_pWorldViewProjVar{ nullptr }, m_pWorldVar { nullptr }, m_pCameraPosVar { nullptr }, 
	m_pUseNormalMapVar{ nullptr }, 
	m_CurrentTech{ 0 }, m_NrTechniques{ 3 }
{
	m_pEffect = LoadEffect(pDevice, assetPath);
	if (m_pEffect)
	{
		// ALWAYS START WITH POINT FILTER 
		std::cout << "SAMPLER_STATE = POINT" << std::endl;

		// Bind with our shader ( Capture necessary variables from the GPU)
		ShaderBinding();
	}
}

// Bind everything needed from the shader with our effect
void Effect::ShaderBinding()
{
	// Techniques
	m_pPointTech = GetTechnique("PointTechnique");
	m_pLinearTech = GetTechnique( "LinearTechnique");
	m_pAnisotropicTech = GetTechnique("AnisotropicTechnique");

	// Matrices
	m_pWorldViewProjVar = GetMatrix("gWorldViewProj");
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

	// Vector variables
	m_pCameraPosVar = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();
	if (!m_pCameraPosVar->IsValid())
	{
		std::cout << "gCameraPosition variable is not valid\n";
	}
}

Effect::~Effect()
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

	if (m_pWorldViewProjVar)
	{
		m_pWorldViewProjVar->Release();
		m_pWorldViewProjVar = nullptr;
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

	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}
}


ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetPath)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };


#if defined( DEBUG ) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetPath.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		std::wstringstream ss;
		if (pErrorBlob != nullptr)
		{
			// Print the error we received
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			ss << "EffectLoader: Failed to createEffectFromFile!\nPath: " << assetPath;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}


	return pEffect;
}

void Effect::ToggleTechnique()
{
	m_CurrentTech++;
	if (m_CurrentTech >= m_NrTechniques)
		m_CurrentTech = 0;

	switch (m_CurrentTech)
	{
	case 0:		// POINT TECHNIQUE
		std::cout << "SAMPLER_STATE = POINT" << std::endl;
		break;
	case 1:		// LINEAR TECHNIQUE
		std::cout << "SAMPLER_STATE = LINEAR" << std::endl;
		break;
	case 2:		// ANISOTROPIC TECHNIQUE
		std::cout << "SAMPLER_STATE = ANISOTROPIC" << std::endl;
		break;
	}
}

void Effect::ToggleNormalMap()
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


void Effect::SetDiffuseMap(const Texture* pDiffuseText)
{
	if (m_pDiffuseMapVar)
		m_pDiffuseMapVar->SetResource(pDiffuseText->GetRSV());
}

void Effect::SetSpecularMap(const Texture* pSpecularText)
{
	if (m_pSpecularMapVar)
		m_pSpecularMapVar->SetResource(pSpecularText->GetRSV());
}
void Effect::SetGlossinessMap(const Texture* pGlossinessText)
{
	if (m_pGlossinessMapVar)
		m_pGlossinessMapVar->SetResource(pGlossinessText->GetRSV());
}

void Effect::SetNormalMap(const Texture* pNormalText)
{
	if (m_pNormalMapVar)
		m_pNormalMapVar->SetResource(pNormalText->GetRSV());
}

void Effect::SetCameraVar(const Vector3& cameraPos)
{
	if (m_pCameraPosVar)
		m_pCameraPosVar->SetFloatVector(reinterpret_cast<const float*>(&cameraPos));
}

ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}

// Return the current technique being used
ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	switch (m_CurrentTech)
	{
	case 0:		// POINT TECHNIQUE
		return m_pPointTech;
	case 1:		// LINEAR TECHNIQUE
		return m_pLinearTech;
	case 2:		// ANISOTROPIC TECHNIQUE
		return m_pAnisotropicTech;
	default:
		return m_pPointTech;
	}
}

ID3DX11EffectMatrixVariable* Effect::GetWorldViewProjMatrix() const
{
	return m_pWorldViewProjVar;
}

ID3DX11EffectMatrixVariable* Effect::GetWorldMatrix() const
{
	return m_pWorldVar;
}

ID3DX11EffectTechnique* Effect::GetTechnique( const std::string& name)
{
	ID3DX11EffectTechnique* technique = m_pEffect->GetTechniqueByName(name.c_str());
	if (!technique->IsValid())
	{
		std::cout << "Technique not valid\n";
		return nullptr;
	}
	return technique;
}

ID3DX11EffectMatrixVariable* Effect::GetMatrix(const std::string& name)
{
	ID3DX11EffectMatrixVariable* matrix = m_pEffect->GetVariableByName(name.c_str())->AsMatrix();
	if (!matrix->IsValid())
	{
		std::cout << name << " not valid\n";
		return nullptr;
	}
	return matrix;
}
ID3DX11EffectShaderResourceVariable* Effect::GetShaderResource(const std::string& name)
{
	ID3DX11EffectShaderResourceVariable* shaderResource = m_pEffect->GetVariableByName(name.c_str())->AsShaderResource();
	
	if (!shaderResource->IsValid())
	{
		std::cout << name << " not valid\n";
		return nullptr;
	}
	return shaderResource;
}