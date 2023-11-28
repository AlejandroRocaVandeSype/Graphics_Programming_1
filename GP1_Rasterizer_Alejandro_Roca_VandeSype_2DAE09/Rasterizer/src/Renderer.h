#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	struct Vertex_Out;
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

		void ToggleFinalColorMode();

	private:

		inline void Render_W1_Part1();
		inline void Render_W1_Part2();
		inline void Render_W1_Part3();
		inline void Render_W1_Part4();
		inline void Render_W1_Part5();
		
		// Week 2
		inline void Render_W2_Part1();	 // QUADS
		inline void Render_W2_Part2();	// Textures & Vertex Attributes
		inline void Render_W2_Part3();	// Depth Interpolation

		// Week 3
		inline void Render_W3();
		inline void Render_W3_Part2();


		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;
		//void VertexTransformationFunction_W2_Part1(std::vector<Mesh>& meshes_in, std::vector<Vertex>& vertices_out) const;
		void VertexTransformationFunction_W2( std::vector<Mesh>& meshes_in) const;
		void VertexTransformationFunction_W3(std::vector<Mesh>& meshes_in) const;

		inline void RenderPixel(const std::vector<dae::Vertex>& vertices_ssv, const std::vector<uint32_t>& meshes_indices, 
			size_t triangleIdx) const;
		inline void RenderPixel_W3(const std::vector<dae::Vertex_Out>& vertices_ssv, const std::vector<uint32_t>& meshes_indices,
			size_t triangleIdx) const;

		inline ColorRGB Remap(float value, float toLow, float toHigh) const;

		Mesh m_TuktukMesh{};

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		Uint32 m_backgroundColor{};

		Texture* m_pTexture;

		float* m_pDepthBufferPixels{};			// Contains the Depth/Z-component from the pixels
		Uint32 m_totalPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};	
		float m_AspectRatio{};

		bool m_UseDepthBufferColor{ false };
	};
}
