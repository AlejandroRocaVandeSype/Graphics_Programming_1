#include "pch.h"
#include "Effect.h"
#include "Texture.h"

using namespace dae;


Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetPath)
	: m_pEffect{ nullptr },
	m_pPointTech{ nullptr }, m_pLinearTech{ nullptr }, m_pAnisotropicTech{ nullptr }, m_pDiffuseMapVariable {nullptr },
	m_CurrentTech { 0 }, m_NrTechniques{ 3 }
{
	m_pEffect = LoadEffect(pDevice, assetPath);

	if (m_pEffect)
	{
		// POINT FILTER AT START
		std::cout << "SAMPLER_STATE = POINT" << std::endl;

		// Binding
		m_pPointTech = m_pEffect->GetTechniqueByName("PointTechnique");
		if (!m_pPointTech->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pLinearTech = m_pEffect->GetTechniqueByName("LinearTechnique");
		if (!m_pLinearTech->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pAnisotropicTech = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
		if (!m_pAnisotropicTech->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pWorldViewProjVariable not valid\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}

	}
}


Effect::~Effect()
{
	if (m_pWorldViewProjVariable)
	{
		m_pWorldViewProjVariable->Release();
		m_pWorldViewProjVariable = nullptr;
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


void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetRSV());
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
	return m_pWorldViewProjVariable;
}
