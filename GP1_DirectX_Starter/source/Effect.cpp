#include "pch.h"
#include "Effect.h"

using namespace dae;


Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetPath)
	: m_pEffect{ nullptr },
	m_pTechnique{ nullptr } 
{
	m_pEffect = LoadEffect(pDevice, assetPath);

	if (m_pEffect)
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pWorldViewProjVariable not valid\n";
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

	if (m_pTechnique)
	{
		m_pTechnique->Release();
		m_pTechnique = nullptr;
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


ID3DX11Effect* Effect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique()
{
	return m_pTechnique;
}

ID3DX11EffectMatrixVariable* Effect::GetWorldViewProjMatrix()
{
	return m_pWorldViewProjVariable;
}