#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <iostream>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		if (!pSurface)
		{
			SDL_Log("Error loading texture: %s", IMG_GetError());
			return nullptr;
		}

		//Create & Return a new Texture Object (using SDL_Surface)
		Texture* pTexture = new Texture(pSurface);

		return pTexture;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		if (!m_pSurface || !m_pSurfacePixels)
		{
			std::cerr << "Texture not loaded or invalid." << std::endl;
			return{};
		}
		
		 // Convert UV coordinates from [0, 1] to [0, width] and [0, height]
		int x{ static_cast<int>(uv.x * (m_pSurface->w - 1)) };
		int y{ static_cast<int>(uv.y * (m_pSurface->h - 1)) };

		/*x = x < 0 ? 0 : x;
		x = x > m_pSurface->w - 1 ? m_pSurface->w - 1 : x;
		y = y < 0 ? 0 : y;
		y = y > m_pSurface->h - 1 ? m_pSurface->h - 1 : y;
		*/
		// With the UV coords we convert them to a single index to be used in the 
		// array of pixels in the surface
		uint32_t index{ static_cast<uint32_t>(y * m_pSurface->w + x) };

		// Get the pixel value from the SDL_SurfacePixels directly
		
		uint32_t pixel{ m_pSurfacePixels[index] };

		// Extract the RGB components using SDL_GetRGB
		uint8_t colorR{}, colorG{}, colorB{};
		SDL_GetRGB(pixel, m_pSurface->format, &colorR, &colorG, &colorB);

		// Remap color to [0, 1] range and return it
		return ColorRGB{ colorR / 255.f, colorG / 255.f, colorB / 255.f };
		
	}
}