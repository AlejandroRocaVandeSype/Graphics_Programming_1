#include "pch.h"
#include "Effect.h"

using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetPath)
	: m_pEffect{ nullptr }, m_pTechnique{ nullptr }, m_pWorldViewProjVar{ nullptr }
{
	m_pEffect = LoadEffect(pDevice, assetPath);

	// Variable binding
	m_pWorldViewProjVar = GetMatrix("gWorldViewProj");

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

Effect::~Effect()
{
	if (m_pWorldViewProjVar)
	{
		m_pWorldViewProjVar->Release();
		m_pWorldViewProjVar = nullptr;
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

void Effect::UpdateWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
{
	m_pWorldViewProjVar->SetMatrix(reinterpret_cast<const float*>(&worldViewProjMatrix));
}


ID3DX11EffectTechnique* Effect::GetTechnique(const std::string& name)
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

ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	return m_pTechnique;
}