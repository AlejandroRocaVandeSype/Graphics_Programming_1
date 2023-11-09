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

 // For multithread ( Parallel execution )
#define PARALLEL_EXECUTION
#include <execution>	

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow)),
	m_CurrentLightingMode{ LightingMode::Combined },
	m_ShadowsEnabled{ true }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_aspectRatio = m_Width / static_cast<float>(m_Height);

	// Calculate the amount of pixels needed based on the width and height 
	uint32_t amountOfPixels{ static_cast<uint32_t>(m_Width * m_Height) };
	m_pixelIndices.reserve(amountOfPixels);

	for (uint32_t index{}; index < amountOfPixels; ++index)
		m_pixelIndices.emplace_back(index);


}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	// Calculate the camera ONB matrix before "shooting" any ray into the scene
	// This way we know in which direction and position the camera is 
	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

#ifdef PARALLEL_EXECUTION
	// Parallel logic

	// Execute renderPixel for each pixel
	std::for_each(std::execution::par, m_pixelIndices.begin(), m_pixelIndices.end(),
		[&](int i)
		{ RenderPixel(pScene, i, cameraToWorld, camera.origin); } );

#else // Synchronous logic (no multithreading)

	for (uint32_t pixelIdx{}; pixelIdx < m_pixelIndices.size(); ++pixelIdx)
	{
		RenderPixel(pScene, pixelIdx, cameraToWorld, camera.origin);
	}
#endif // PARALLEL_EXECUTION

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);

}


void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, const Matrix& cameraToWorld, const Vector3& cameraOrigin) const
{
	std::vector<dae::Material*> materials{ pScene->GetMaterials() };

	const uint32_t px{ pixelIndex % m_Width };
	const uint32_t py{ pixelIndex / m_Width };

	// For each pixel
			//... Ray calculation ( Take aspect ratio and FOV into account )
			//... Add half of the pixel size to get the center of the pixel
	Vector3 rayDirection{};
	rayDirection.x = (2.f * ((static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width)) - 1.f) * m_aspectRatio 
		* pScene->GetCamera().fov;
	rayDirection.y = (1.f - 2.f * (static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height)) * pScene->GetCamera().fov;
	rayDirection.z = 1.f;

	// Transform this ray direction using the Camera ONB matrix, so we take into account 
	// the camera rotation / position
	rayDirection = cameraToWorld.TransformVector(rayDirection).Normalized();

	// Ray we are casting from the camera towards each pixel
	Ray viewRay{ cameraOrigin , rayDirection };

	// Color to write to the color buffer ( default = black)
	ColorRGB finalColor{};

	// HitRecord containing more info about potential hit
	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	// SHADING 
	if (closestHit.didHit)
	{
		for (size_t index{ 0 }; index < pScene->GetLights().size(); ++index)
		{
			// Light direction ( From point to light)
			Vector3 lightDirection{ LightUtils::GetDirectionToLight(pScene->GetLights()[index], closestHit.origin) };

			// ** LAMBERT'S COSINE LAW ** -> Measure the OBSERVED AREA
			const float viewAngle{ Vector3::Dot(closestHit.normal, lightDirection.Normalized()) };
			if (viewAngle < 0 && m_CurrentLightingMode != LightingMode::Radiance
				&& m_CurrentLightingMode != LightingMode::BRDF)
			{
				// If it is below 0 the point on the surface points away from the light
				// ( It doesn't contribute for the finalColor)
				// Skip to the next light
				continue;
			}

			// ** SHADOWS ** 
			if (m_ShadowsEnabled)
			{
				// Small offset to avoid self-shadowing
				Vector3 originOffset{ closestHit.origin + (closestHit.normal * 0.001f) };

				// Ray from the closestHit towards the light
				Ray lightRay{ originOffset , Vector3{LightUtils::GetDirectionToLight(pScene->GetLights()[index], originOffset)} };

				// Max of the ligh ray will be its own magnitude
				lightRay.max = lightRay.direction.Magnitude();
				lightRay.direction = lightRay.direction.Normalized();

				// Check if the ray hits
				if (pScene->DoesHit(lightRay))
				{
					// Shadowed -> Skip next color
					continue;
				}
			}

			// ** LIGHT SCATTERING ** based on the material from the objects from the scene
			const ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, lightDirection.Normalized(), viewRay.direction) };

			//finalColor += BRDF;			// BRDF ONLY
			// ** LIGHTING EQUATION **
			 //FinalColor calculated based on the current Lighting Mode
			switch (m_CurrentLightingMode)
			{
			case dae::Renderer::LightingMode::ObservedArea:
				finalColor += ColorRGB{ viewAngle, viewAngle, viewAngle }; // ObservedArea Only 
				break;
			case dae::Renderer::LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[index], closestHit.origin); // Incident Radiance Only
				break;
			case dae::Renderer::LightingMode::BRDF:
				finalColor += BRDF;			// BRDF ONLY
				break;
			case dae::Renderer::LightingMode::Combined:
				finalColor += LightUtils::GetRadiance(pScene->GetLights()[index], closestHit.origin) * BRDF * viewAngle;
				break;
			}

		}
	}

	//Update Color in Buffer 
	finalColor.MaxToOne(); // Clamp final color to prevent color overflow
	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}


void Renderer::ToggleShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
		case dae::Renderer::LightingMode::ObservedArea:
			std::cout << "LIGHTING MODE : Incident Radiance " << std::endl;
			m_CurrentLightingMode = LightingMode::Radiance;
			break;
		case dae::Renderer::LightingMode::Radiance:
			std::cout << "LIGHTING MODE : BRDF" << std::endl;
			m_CurrentLightingMode = LightingMode::BRDF;
			break;
		case dae::Renderer::LightingMode::BRDF:
			std::cout << "LIGHTING MODE : Combined" << std::endl;
			m_CurrentLightingMode = LightingMode::Combined;
			break;
		case dae::Renderer::LightingMode::Combined:
			std::cout << "LIGHTING MODE : Observed Area" << std::endl;
			m_CurrentLightingMode = LightingMode::ObservedArea;
			break;
	}
}
