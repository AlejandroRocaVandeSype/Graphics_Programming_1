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
#include <iostream>

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

	// Calculate the camera ONB matrix before "shooting" any ray into the scene
	// This way we know in which direction and position the camera is 
	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	// Current aspect ratio
	float aspectRatio{ m_Width / static_cast<float>(m_Height) };
	// To get the center of the pixel
	const float halfPixelSize{ 0.5f };
	// Ray origin (Camera)
	const Vector3 origin{ 0, 0, 0 };

	// Calculate the FOV for the ray calculation
	// (Could be better optimized if we only calculated when it changes
	// instead of every frame )
	const float FOV{ tan( (camera.fovAngle * TO_RADIANS) / 2) };
	
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// For each pixel
			//... Ray calculation ( Take aspect ratio and FOV into account )
			Vector3 rayDirection{};
			rayDirection.x = (2.f * ((static_cast<float>(px) + halfPixelSize) / static_cast<float>(m_Width)) - 1.f) * aspectRatio * FOV;
			rayDirection.y = (1.f - 2.f * (static_cast<float>(py) + halfPixelSize) / static_cast<float>(m_Height) ) * FOV;
			rayDirection.z = 1.f;

			// Transform this ray direction using the Camera ONB matrix, so we take into account 
			// the camera rotation / position
			rayDirection = cameraToWorld.TransformVector(rayDirection).Normalized();

			// Ray we are casting from the camera towards each pixel
			Ray viewRay{ camera.origin , rayDirection};

			// Color to write to the color buffer ( default = black)
			ColorRGB finalColor{};

			// HitRecord containing more info about potential hit
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			// SHADING 
			if (closestHit.didHit)
			{
				// If we hit something, set finalColor to material color, else keep black
				// Use HitRecord::materialIndex to find the corresponding material
				finalColor = materials[closestHit.materialIndex]->Shade();

				// Check if the pixel is shadowed
				for (size_t index{ 0 }; index < lights.size(); ++index)
				{
					// Small offset to avoid self-shadowing
					Vector3 originOffset{ closestHit.origin + (closestHit.normal * 0.001f) };

					// Ray from the closestHit towards the light
					Ray lightRay{ originOffset , LightUtils::GetDirectionToLight(lights[index], originOffset) };

					// Max of the ligh ray will be its own magnitude
					lightRay.max = lightRay.direction.Magnitude();
					lightRay.direction = lightRay.direction.Normalized();
				
					// Check if the ray hits
					if (pScene->DoesHit(lightRay))
					{
						// Darken color
						finalColor *= 0.5f;
					}
				}

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
