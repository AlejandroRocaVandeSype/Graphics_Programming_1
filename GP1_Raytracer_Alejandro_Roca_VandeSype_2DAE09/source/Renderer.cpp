//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// (px, py, 1) Ray to the front
			//		Subtract / add to px/py to move our ray around the scene
			//		(Half of the size of our screen to add/subtract)
			//		Ray goes to left-up corner, need to go center pixel (add half size pixel)

			// Current aspect ratio
			float aspectRatio{ m_Width / static_cast<float>(m_Height)};

			// To get the center of the pixel
			const float halfPixelSize{ 0.5f };

			Vector3 rayDirection{};
			rayDirection.z = 1.f;
			rayDirection.x = (2.f * (( static_cast<float>(px) + halfPixelSize ) / static_cast<float>(m_Width)) - 1.f ) * aspectRatio;
			rayDirection.y = 1.f - 2.f * (static_cast<float>(py) + halfPixelSize) / static_cast<float>(m_Height);

			rayDirection.Normalize();

			const Vector3 origin{ 0, 0, 0 };
			Ray hitRay{ origin , rayDirection};

			/*float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ gradient, gradient, gradient };*/

			ColorRGB finalColor{ rayDirection.x, rayDirection.y, rayDirection.z };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
