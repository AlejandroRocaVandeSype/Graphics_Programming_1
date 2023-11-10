#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;

	private:

		inline void Render_W1_Part1();
		inline void Render_W1_Part2();
		inline void Render_W1_Part3();
		inline void Render_W1_Part4();
		inline void Render_W1_Part5();

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		Uint32 m_backgroundColor{};

		float* m_pDepthBufferPixels{};			// Contains the Depth/Z-component from the pixels
		Uint32 m_totalPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};	
		float m_AspectRatio{};
	};
}
