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

	// Current aspect ratio
	float aspectRatio{ m_Width / static_cast<float>(m_Height) };
	// To get the center of the pixel
	const float halfPixelSize{ 0.5f };
	// Ray origin (Camera)
	const Vector3 origin{ 0, 0, 0 };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// For each pixel
			//... Ray calculation
			Vector3 rayDirection{};
			rayDirection.x = (2.f * (( static_cast<float>(px) + halfPixelSize ) / static_cast<float>(m_Width)) - 1.f ) * aspectRatio;
			rayDirection.y = 1.f - 2.f * (static_cast<float>(py) + halfPixelSize) / static_cast<float>(m_Height);
			rayDirection.z = 1.f;
			rayDirection.Normalize();

			// Ray we are casting from the camera towards each pixel
			Ray viewRay{ origin , rayDirection};

			// Color to write to the color buffer ( default = black)
			ColorRGB finalColor{};

			// HitRecord containing more info about potential hit
			HitRecord closestHit{};

			//TEMP SPHERE (default material = solid red - id = 0)
			//................ origin, radius and material Index
			Sphere testSphere{ {0.f, 0.f, 100.f}, 50.f , 0 };

			// Perform Sphere HitTest
			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				// If we hit something, set finalColor to material color, else keep black
				// Use HitRecord::materialIndex to find the corresponding material
				finalColor = materials[closestHit.materialIndex]->Shade();


				// Verify t-values
				// Remap t-value to [0, 1] (should lay between ~[50,90]
				/*const float scaled_t{ (closestHit.t - 50.f) / 40.f };
				finalColor = { scaled_t, scaled_t, scaled_t };*/
			}

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
