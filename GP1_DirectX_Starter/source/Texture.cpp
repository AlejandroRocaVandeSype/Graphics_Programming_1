#include "pch.h"
#include "Texture.h"

using namespace dae;

Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface) :
	m_pResource{ nullptr},
	m_pSRV{ nullptr }
{

	// Push the date from the SDL_SURFACE to the resource
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if (FAILED(hr))
	{
		SDL_FreeSurface(pSurface);
		pSurface = nullptr;
		return;
	}

	// Create Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
	
	// SDL_SURFACE is not longer needed. Data is in the resource now
	SDL_FreeSurface(pSurface);
	pSurface = nullptr;
	
}

Texture::~Texture()
{
	if (m_pSRV)
	{
		m_pSRV->Release();
		m_pSRV = nullptr;
	}

	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}

}


Texture* Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
{
	//Load SDL_Surface using IMG_LOAD
	SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
	if (!pSurface)
	{
		SDL_Log("Error loading texture: %s", IMG_GetError());
		return nullptr;
	}

	//Create & Return a new Texture Object (using SDL_Surface)
	Texture* pTexture = new Texture(pDevice, pSurface);

	return pTexture;
}

ID3D11ShaderResourceView* Texture::GetRSV() const
{
	return m_pSRV;
}