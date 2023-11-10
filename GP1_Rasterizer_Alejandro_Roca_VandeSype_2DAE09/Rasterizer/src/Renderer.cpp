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
	m_backgroundColor = SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100);

	// Create the depth buffer with the amount of pixels
	m_totalPixels = m_Width * m_Height;
	m_pDepthBufferPixels = new float[m_totalPixels];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

	m_AspectRatio = m_Width / static_cast<float>(m_Height);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
		// Init with max value of float ( Represents points that are inf far away)
	std::fill_n(m_pDepthBufferPixels, m_totalPixels, FLT_MAX);
	
	// Clear backBuffer ( Redraw background color )
	SDL_FillRect(m_pBackBuffer, NULL, m_backgroundColor);
	
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//Render_W1_Part1();		// Rasterizer Stage Only	
	//Render_W1_Part2();		// Projection Stage	( Camera )
	//Render_W1_Part3();		// Barycentric Coordinates
	//Render_W1_Part4();		// Depth Buffer
	Render_W1_Part5();			// BoundingBox Optimization

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

inline void Renderer::Render_W1_Part3()
{
	// Define triangle - Vertices in World Space
	std::vector<Vertex> vertices_world
	{
		{ {0.f, 4.f, 2.f}, ColorRGB{1,0, 0} },
		{ {3.f, -2.f, 2.f}, ColorRGB{0,1, 0} },
		{ {-3.f, -2.f, 2.f}, ColorRGB{0,0, 1} }
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
		screenSpaceVertex.color = vertex.color;
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

				// Convert to Vector2
				Vector2 v0{ vertices_ssv[vertex].position.x, vertices_ssv[vertex].position.y };
				Vector2 v1{ vertices_ssv[vertex+1].position.x, vertices_ssv[vertex+1].position.y };
				Vector2 v2{ vertices_ssv[vertex+2].position.x, vertices_ssv[vertex+2].position.y };

				//// Pixel inside the triangle? 
				////.... Check for every edge from the triangle if the point is on the right side
				Vector2 edge{};
				Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle
				bool isInside{ true };	// Pixel inside triangle?

				// Baycentric Coordinates
				// ... Store the results from the Cross Products (Weights)
				float w0{}, w1{}, w2{};
				float totalWeight{};

				// Weight of V0
				toPoint = pixel - v1;
				edge = v2 - v1;
				w0 = Vector2::Cross(edge, toPoint);
				if (w0 < 0)
					isInside = false;

				if (isInside)
				{
					// Weight of V1
					toPoint = pixel - v2;
					edge = v0 - v2;
					w1 = Vector2::Cross(edge, toPoint);
					if (w1 < 0)
						isInside = false;

					if (isInside)
					{
						// Weight of V2
						toPoint = pixel - v0;
						edge = v1 - v0;
						w2 = Vector2::Cross(edge, toPoint);
						if (w2 < 0)
							isInside = false;

						if (isInside)
						{
							totalWeight += w0 + w1 + w2;

							// Calculate the final weights by dividing with 
							// the total area of the parallelogram
							w0 = w0 / totalWeight;
							w1 = w1 / totalWeight;
							w2 = w2 / totalWeight;

							// Final color of the triangle is gonna be an interpolated color based on the color 
							// and weight from the vertices
							finalColor = (vertices_ssv[vertex].color * w0) + (vertices_ssv[vertex + 1].color * w1)
								+ (vertices_ssv[vertex + 2].color * w2);
						}		
					}			
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


inline void Renderer::Render_W1_Part4()
{
	// Define triangle - Vertices in World Space
	std::vector<Vertex> vertices_world
	{
		// Triangle 0 ( In front )
		{ {0.f, 2.f, 0.f}, ColorRGB{1,0, 0} },
		{ {1.5f, -1.f, 0.f}, ColorRGB{1,0, 0} },
		{ {-1.5f, -1.f, 0.f}, ColorRGB{1,0, 0} },

		// Triangle 1 ( Back)
		{ {0.f, 4.f, 2.f}, ColorRGB{1,0, 0} },
		{ {3.f, -2.f, 2.f}, ColorRGB{0,1, 0} },
		{ {-3.f, -2.f, 2.f}, ColorRGB{0,0, 1} }
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
		screenSpaceVertex.position.z = vertex.position.z;
		screenSpaceVertex.color = vertex.color;
		vertices_ssv.emplace_back(screenSpaceVertex);
	}

	// Loop through all triangles ( Every 3 vertices == 1 triangle )
	for (size_t vertex{ 0 }; vertex < vertices_ssv.size(); vertex += 3)
	{
		Vector2 pixel{};
		// Convert to Vector2
		Vector2 v0{ vertices_ssv[vertex].position.x, vertices_ssv[vertex].position.y };
		Vector2 v1{ vertices_ssv[vertex + 1].position.x, vertices_ssv[vertex + 1].position.y };
		Vector2 v2{ vertices_ssv[vertex + 2].position.x, vertices_ssv[vertex + 2].position.y };

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

				//// Pixel inside the triangle? 
				////.... Check for every edge from the triangle if the point is on the right side
				Vector2 edge{};
				Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle
				
				// Baycentric Coordinates
				// ... Store the results from the Cross Products (Weights)
				float w0, w1, w2;
				float totalWeight;

				// Weight of V0
				toPoint = pixel - v1;
				edge = v2 - v1;
				w0 = Vector2::Cross(edge, toPoint);
				if (w0 < 0)
					continue;	// Pixel not inside triangle -> Go next pixel
				
				// Weight of V1
				toPoint = pixel - v2;
				edge = v0 - v2;
				w1 = Vector2::Cross(edge, toPoint);
				if (w1 < 0)
					continue;

				// Weight of V2
				toPoint = pixel - v0;
				edge = v1 - v0;
				w2 = Vector2::Cross(edge, toPoint);
				if (w2 < 0)
					continue;
			
				totalWeight = w0 + w1 + w2;

				// Calculate the final weights by dividing with 
				// the total area of the parallelogram
				w0 = w0 / totalWeight;
				w1 = w1 / totalWeight;
				w2 = w2 / totalWeight;

				// DEPTH TEST 
				// ... Check if pixel is closer than the one in the depth buffer	
				int pixelIndex{ px + (py * m_Width) };

				// Use the barycentric weights to calcualte the interpolated depth of the pixel inside
				float pixelDepth{ (w0 * vertices_ssv[vertex].position.z) + (w1 * vertices_ssv[vertex + 1].position.z) +
					(w2 * vertices_ssv[vertex + 2].position.z) };

				if (pixelDepth >= m_pDepthBufferPixels[pixelIndex])
					 continue;  // Pixel further away. Don't render it
					
				// This pixel is closer -> Render the color of this one
				// Store it
				m_pDepthBufferPixels[pixelIndex] = pixelDepth;

				// Final color of the triangle is gonna be an interpolated color based on the color 
				// and weight from the vertices
				finalColor = (vertices_ssv[vertex].color * w0) + (vertices_ssv[vertex + 1].color * w1)
								+ (vertices_ssv[vertex + 2].color * w2);
				

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

inline void Renderer::Render_W1_Part5()
{
	// Define triangle - Vertices in World Space
	std::vector<Vertex> vertices_world
	{
		// Triangle 0 ( In front )
		{ {0.f, 2.f, 0.f}, ColorRGB{1,0, 0} },
		{ {1.5f, -1.f, 0.f}, ColorRGB{1,0, 0} },
		{ {-1.5f, -1.f, 0.f}, ColorRGB{1,0, 0} },

		// Triangle 1 ( Back)
		{ {0.f, 4.f, 2.f}, ColorRGB{1,0, 0} },
		{ {3.f, -2.f, 2.f}, ColorRGB{0,1, 0} },
		{ {-3.f, -2.f, 2.f}, ColorRGB{0,0, 1} }
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
		screenSpaceVertex.position.z = vertex.position.z;
		screenSpaceVertex.color = vertex.color;
		vertices_ssv.emplace_back(screenSpaceVertex);
	}

	// Loop through all triangles ( Every 3 vertices == 1 triangle )
	for (size_t vertex{ 0 }; vertex < vertices_ssv.size(); vertex += 3)
	{
		Vector2 pixel{};

		// Convert to Vector2
		Vector2 v0{ vertices_ssv[vertex].position.x, vertices_ssv[vertex].position.y };
		Vector2 v1{ vertices_ssv[vertex + 1].position.x, vertices_ssv[vertex + 1].position.y };
		Vector2 v2{ vertices_ssv[vertex + 2].position.x, vertices_ssv[vertex + 2].position.y };

		// BoundingBox Optimization ( For each triangle )
		// ... Iterate only over the pixels defined by the boundingbox

		// Top-Left ( Smallest x-value)
		// Right-Bottom ( ( Highest y-value& xValue )
		int minX{}, maxX{};
		int minY{}, maxY{};

		minX = std::min(std::min(v0.x, v1.x), v2.x);
		maxX = std::max(std::max(v0.x, v1.x), v2.x);
		minY = std::min(std::min(v0.y, v1.y), v2.y);
		maxY = std::max(std::max(v0.y, v1.y), v2.y);

		// Make sure boundaries don't exceed screen boundaries
		minX = minX < 0 ? 0 : minX;
		maxX = maxX > m_Width ? m_Width : maxX;
		minY = minY < 0 ? 0 : minY;
		maxY = maxY > m_Height ? m_Height : maxY;

		// Loop only through the pixels that Bounding box cover
		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				// Center pixel
				pixel.x = px + 0.5f;
				pixel.y = py + 0.5f;

				// Black color for pixels outside triangle
				ColorRGB finalColor{ colors::Black };

				//// Pixel inside the triangle? 
				////.... Check for every edge from the triangle if the point is on the right side
				Vector2 edge{};
				Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle

				// Baycentric Coordinates
				// ... Store the results from the Cross Products (Weights)
				float w0, w1, w2;
				float totalWeight;

				// Weight of V0
				toPoint = pixel - v1;
				edge = v2 - v1;
				w0 = Vector2::Cross(edge, toPoint);
				if (w0 < 0)
					continue;	// Pixel not inside triangle -> Go next pixel

				// Weight of V1
				toPoint = pixel - v2;
				edge = v0 - v2;
				w1 = Vector2::Cross(edge, toPoint);
				if (w1 < 0)
					continue;

				// Weight of V2
				toPoint = pixel - v0;
				edge = v1 - v0;
				w2 = Vector2::Cross(edge, toPoint);
				if (w2 < 0)
					continue;

				totalWeight = w0 + w1 + w2;

				// Calculate the final weights by dividing with 
				// the total area of the parallelogram
				w0 = w0 / totalWeight;
				w1 = w1 / totalWeight;
				w2 = w2 / totalWeight;

				// DEPTH TEST 
				// ... Check if pixel is closer than the one in the depth buffer	
				int pixelIndex{ px + (py * m_Width) };

				// Use the barycentric weights to calcualte the interpolated depth of the pixel inside
				float pixelDepth{ (w0 * vertices_ssv[vertex].position.z) + (w1 * vertices_ssv[vertex + 1].position.z) +
					(w2 * vertices_ssv[vertex + 2].position.z) };

				if (pixelDepth > m_pDepthBufferPixels[pixelIndex])
					continue;  // Pixel further away. Don't render it

				// This pixel is closer -> Render the color of this one
				// Store it
				m_pDepthBufferPixels[pixelIndex] = pixelDepth;

				// Final color of the triangle is gonna be an interpolated color based on the color 
				// and weight from the vertices
				finalColor = (vertices_ssv[vertex].color * w0) + (vertices_ssv[vertex + 1].color * w1)
					+ (vertices_ssv[vertex + 2].color * w2);

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
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
		projectedVertex.color = vertex.color;

		// Now our vertex is in NDC(Space)
		vertices_out.emplace_back(projectedVertex);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
