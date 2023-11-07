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
	m_pWindow(pWindow),
	m_AspectRatio{ -1.f }
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

	m_AspectRatio = m_Width / static_cast<float>(m_Height);
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

	//Render_W1_Part1();		// Rasterizer Stage Only	
	Render_W1_Part2();		// Projection Stage	( Camera )

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}


inline void Renderer::Render_W1_Part1()
{
	// Define Triangle - Vertices in NDC ( Normalized Device Coord ) space 
	std::vector<Vertex> vertices_ndc
	{
		{{0.f, 0.5f, 1.f}},    // V0
		{{0.5f, -0.5f, 1.f}},  // V1
		{{-0.5f, -0.5f, 1.f}}  // V2
	};

	// Convert coordinates from NDC to Screen Space 
	std::vector<Vertex> vertices_ssv{};
	vertices_ssv.reserve(vertices_ndc.size());
	Vertex screenSpaceVertex{};
	for (const Vertex& vertex : vertices_ndc)
	{
		screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
		screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
		screenSpaceVertex.position.z = 1.f;
		vertices_ssv.emplace_back(screenSpaceVertex);
	}


	// Loop through all triangles ( Every 3 vertices == 1 triangle )
	for (size_t vertex { 0 }; vertex < vertices_ssv.size(); vertex += 3)
	{
		Vector2 pixel{};
		// Loop through all pixels 
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				// Center pixel
				pixel.x = px + 0.5f;
				pixel.y = py + 0.5f;

			// Black color for pixels outside triangle
			ColorRGB finalColor{ colors::Black }; 

			if (GeometryUtils::Triangle_PixelTest(pixel, vertices_ssv[vertex].position, vertices_ssv[vertex + 1].position,
					vertices_ssv[vertex + 2].position) == true)
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

inline void Renderer::Render_W1_Part2()
{
	// Define triangle - Vertices in World Space
	std::vector<Vertex> vertices_world
	{
		{ {0.f, 2.f, 0.f} },
		{ {1.f, 0.f, 0.f} },
		{ {-1.f, 0.f, 0.f} }
	};
	
	// *** PROJECTION STAGE ***
	// Transform the vector with the World Space vertices to NDC space vertices
	std::vector<Vertex> vertices_ndc{};
	VertexTransformationFunction(vertices_world, vertices_ndc);
	
	// *** RASTERIZATION STAGE ***
	// Convert coordinates from NDC to Screen Space 
	std::vector<Vertex> vertices_ssv{};
	vertices_ssv.reserve(vertices_ndc.size());
	Vertex screenSpaceVertex{};
	for (const Vertex& vertex : vertices_ndc)
	{
		screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
		screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
		screenSpaceVertex.position.z = 1.f;
		vertices_ssv.emplace_back(screenSpaceVertex);
	}

	// Loop through all triangles ( Every 3 vertices == 1 triangle )
	// Loop through all triangles ( Every 3 vertices == 1 triangle )
	for (size_t vertex{ 0 }; vertex < vertices_ssv.size(); vertex += 3)
	{
		Vector2 pixel{};
		// Loop through all pixels 
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				// Center pixel
				pixel.x = px + 0.5f;
				pixel.y = py + 0.5f;

				// Black color for pixels outside triangle
				ColorRGB finalColor{ colors::Black };

				if (GeometryUtils::Triangle_PixelTest(pixel, vertices_ssv[vertex].position, vertices_ssv[vertex + 1].position,
					vertices_ssv[vertex + 2].position) == true)
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


	Vertex viewSpaceVertex{};
	Vertex projectedVertex{};
	vertices_out.reserve(vertices_in.size());
	for (const auto& vertex : vertices_in)
	{
		// Multiply each vertex with the ViewMatrix -> Transform to View(Space)
		viewSpaceVertex.position = m_Camera.viewMatrix.TransformPoint(vertex.position);

		// Vertices are not in NDC -> Transform them to NDC(Space)
		// ... Apply Perspective Divide to each projected point
		projectedVertex.position.x = viewSpaceVertex.position.x / viewSpaceVertex.position.z;
		projectedVertex.position.y = viewSpaceVertex.position.y / viewSpaceVertex.position.z;
		projectedVertex.position.z = viewSpaceVertex.position.z;

		// ... Apply Camera Settings & Screen size
		projectedVertex.position.x = projectedVertex.position.x / (m_Camera.fov * m_AspectRatio);
		projectedVertex.position.y = projectedVertex.position.y / m_Camera.fov;

		// Now our vertex is in NDC(Space)
		vertices_out.emplace_back(projectedVertex);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
