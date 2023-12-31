#pragma once

#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Surface;


namespace dae
{
	class Scene;
	struct Matrix;
	struct Vector3;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, const Matrix& cameraToWorld, const Vector3& cameraOrigin) const;	  // Process each pixel
		bool SaveBufferToImage() const;

		// LIGHTING
		void CycleLightingMode();
		void ToggleShadows();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{}; // This will change every frame

		int m_Width{};
		int m_Height{};
		float m_aspectRatio{};
		std::vector<uint32_t> m_pixelIndices{};

		// LIGHTING
		enum class LightingMode
		{
			ObservedArea,		// Lambert Cosine Law
			Radiance,			// Incident Radiance
			BRDF,				// Scattering of the Light
			Combined			// ObservedArea * Radiance * BRDF
		};

		LightingMode m_CurrentLightingMode;
		bool m_ShadowsEnabled;

	};
}
