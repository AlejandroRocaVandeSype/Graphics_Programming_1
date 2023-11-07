//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	Render_W1_Part1();		// Rasterizer Stage Only	

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}


inline void Renderer::Render_W1_Part1()
{
	// Define Triangle - Vertices in NDC ( Normalized Device Coord ) space 
	std::vector<Vector3> vertices_ndc
	{
		{0.f, 0.5f, 1.f},
		{0.5f, -0.5f, 1.f},
		{-0.5f, -0.5f, 1.f}
	};

	// Convert coordinates from NDC to Screen Space 
	std::vector<Vector3> vertices_ssv{};
	vertices_ssv.reserve(vertices_ndc.size());
	Vector3 screenSpaceVertex{};
	for (const Vector3& vertex : vertices_ndc)
	{
		screenSpaceVertex.x = ( (vertex.x + 1) / 2 ) * m_Width;
		screenSpaceVertex.y = ((1 - vertex.y) / 2) * m_Height;
		screenSpaceVertex.z = 1.f;
		vertices_ssv.emplace_back(screenSpaceVertex);
	}

	// Loop through all triangles
	for (size_t vertex { 0 }; vertex < vertices_ssv.size(); vertex += 3)
	{
		Vector3 pixel{};
		// Loop through all pixels 
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				// Center pixel
				pixel.x = px + 0.5f;
				pixel.y = py + 0.5f;
				pixel.z = 1.f;

				ColorRGB finalColor{ colors::Black };

				if (GeometryUtils::PixelTest_Triangle(pixel, vertices_ssv[vertex], vertices_ssv[vertex + 1],
					vertices_ssv[vertex + 2]) == true)
				{
					// Pixel inside triangle -> White color
					finalColor = colors::White;
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}

	
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
