#include "pch.h"
#include "FireEffect.h"
#include "Texture.h"

using namespace dae;

FireEffect::FireEffect(ID3D11Device* pDevice, const std::wstring& assetPath)
	:Effect(pDevice, assetPath)
	, m_pDiffuseMapVar{ nullptr }
{
	// Shader Resources
	m_pDiffuseMapVar = GetShaderResource("gDiffuseMap");
	m_pTechnique = Effect::GetTechnique("DefaultTechnique");
}

void FireEffect::SetMaps(const std::unordered_map<std::string, Texture*>& textures)
{
	std::string key{ "diffuse" };
	auto textureItr = textures.find(key);
	if (textureItr != textures.end())
	{
		m_pDiffuseMapVar->SetResource(textureItr->second->GetRSV());
	}
	else
	{
		std::cout << "Error! Couldn't find " + key + " texture. \n";
	}

}


FireEffect::~FireEffect()
{

	if (m_pDiffuseMapVar)
	{
		m_pDiffuseMapVar->Release();
		m_pDiffuseMapVar = nullptr;
	}
}