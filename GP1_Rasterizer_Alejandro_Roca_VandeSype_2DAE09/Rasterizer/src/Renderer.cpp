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
	m_AspectRatio{ -1.f },
	m_pTexture{ nullptr }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_backgroundColor = SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100);

	// Load texture from the file
	//m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");
	//m_pTexture = Texture::LoadFromFile("Resources/tuktuk.png");
	m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");

	// Create the depth buffer with the amount of pixels
	m_totalPixels = m_Width * m_Height;
	m_pDepthBufferPixels = new float[m_totalPixels];

	m_AspectRatio = m_Width / static_cast<float>(m_Height);

	//Initialize Camera
	m_Camera.Initialize(m_AspectRatio, 45.f, { 0.f,0.f, 0.f });


	m_TuktukMesh.primitiveTopology = PrimitiveTopology::TriangleList;
	Utils::ParseOBJ("Resources/vehicle.obj", m_TuktukMesh.vertices, m_TuktukMesh.indices);
	m_TuktukMesh.Translate(Vector3{ 0.f, 0.f, 50.f });
	
	// World ( Where my meshes are gonna be placed) - View ( Move my meshes to be oriented towards the camera) -
	// Projection ( Squash them for the NDC)  --> Rasterize process -> stretch my pixels

	// Now pixel shader. Our previous vertices_out become vertices_in for the pixel shader
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
	m_TuktukMesh.RotateY((PI_DIV_2/ 2.f) * pTimer->GetTotal());
	m_TuktukMesh.UpdateTransforms();

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
	//Render_W1_Part5();		// BoundingBox Optimization
	  
	//Render_W2_Part1();
	//Render_W2_Part2();			// Textures & Vertex Attributes
	//Render_W2_Part3();

	//Render_W3();				// Only square with Texture
	Render_W3_Part2();			// With TukTuk

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

	std::vector<Uint32> trianglesIndices{ 0, 1, 2, 3, 4, 5 };

	// Loop through all triangles ( Every 3 indeces == 1 triangle )
	for (size_t index{ 0 }; index < trianglesIndices.size(); index += 3)
	{
		Vector2 pixel{};

		// Convert to Vector2
		Vector2 v0{ vertices_ssv[trianglesIndices.at(index)].position.x, vertices_ssv[trianglesIndices.at(index)].position.y };
		Vector2 v1{ vertices_ssv[trianglesIndices.at(index + 1)].position.x, vertices_ssv[trianglesIndices.at(index + 1)].position.y };
		Vector2 v2{ vertices_ssv[trianglesIndices.at(index + 2)].position.x, vertices_ssv[trianglesIndices.at(index + 2)].position.y };

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
		// ... Max are a little big bigger to avoid spots between multiple boundingbox 
		// when they are close to each other ( Black lines )
		minX = minX < 0 ? 0 : minX;
		maxX = maxX > m_Width ? m_Width : maxX + 5;
		minY = minY < 0 ? 0 : minY;
		maxY = maxY > m_Height ? m_Height : maxY + 5;

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
					continue;	// Pixel NOT inside triangle -> Go to next pixel

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

				// Pixel INSIDE triangle !
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
				float pixelDepth{ (w0 * vertices_ssv[trianglesIndices.at(index)].position.z) + (w1 * vertices_ssv[trianglesIndices.at(index + 1)].position.z) +
					(w2 * vertices_ssv[trianglesIndices.at(index + 2)].position.z) };

				if (pixelDepth > m_pDepthBufferPixels[pixelIndex])
					continue;  // Pixel further away. Don't render it

				// This pixel is closer -> Render the color of this one
				// Store it
				m_pDepthBufferPixels[pixelIndex] = pixelDepth;

				// Final color of the triangle is gonna be an interpolated color based on the color 
				// and weight from the vertices
				finalColor = (vertices_ssv[trianglesIndices.at(index)].color * w0) + (vertices_ssv[trianglesIndices.at(index + 1)].color * w1)
					+ (vertices_ssv[trianglesIndices.at(index + 2)].color * w2);

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

inline void Renderer::Render_W2_Part1()
{
	
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{ // Vertices
				Vertex{ {-3.f, 3.f, -2.f} },
				Vertex{ {0.f, 3.f, -2.f} },
				Vertex{ {3.f, 3.f, -2.f} },
				Vertex{ {-3.f, 0.f, -2.f} },
				Vertex{ {0.f, 0.f, -2.f} },
				Vertex{ {3.f, 0.f, -2.f} },
				Vertex{ {-3.f, -3.f, -2.f} },
				Vertex{ {0.f, -3.f, -2.f} },
				Vertex{ {3.f, -3.f, -2.f} },
			},
			
			//{ // Indices ( TriangleList )
			//	3, 0, 1,	1, 4, 3,   4, 1, 2,
			//	2, 5, 4,	6, 3, 4,	4, 7, 6,
			//	7, 4, 5,	5, 8, 7
			//},
			{ // Indices ( TriangleStrip )
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};


	// *** PROJECTION STAGE ***
	// Transform the vector with the World Space vertices to NDC space vertices
	std::vector<Vertex> vertices_ndc{};
	/*VertexTransformationFunction_W2(meshes_world, vertices_ndc);*/

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

	std::vector<Uint32> meshes_indices{ meshes_world.at(0).indices };

	if (meshes_world.at(0).primitiveTopology == PrimitiveTopology::TriangleList)
	{
		// Loop through all triangles ( Every 3 indeces is one triangle )
		for (size_t triangleIdx{ 0 }; triangleIdx < meshes_indices.size(); triangleIdx += 3)
		{
			RenderPixel(vertices_ssv, meshes_indices, triangleIdx);
		}
	}
	else
	{
		// TriangleStrip Mode
		// Loop through all triangles ( Loop shifting varies depending on our PrimitiveTopology mode )
		for (size_t triangleIdx{ 0 }; triangleIdx + 2 < meshes_indices.size(); ++triangleIdx)
		{
			if (triangleIdx % 2 != 0)
			{
				// Odd triangle -> Swap the last two vertices
				std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
			}

			// If no surface area ( Two identical indeces ) then we are in a degenerate triangle
			if (meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 1] ||
				meshes_indices[triangleIdx + 1] == meshes_indices[triangleIdx + 2]
				|| meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 2])
			{
				if (triangleIdx % 2 != 0)
				{
					// Swap back to original
					std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
				}
				// Degenerate triangle -> Go next one
				continue;
			}

			RenderPixel(vertices_ssv, meshes_indices, triangleIdx);

			if (triangleIdx % 2 != 0)
			{
				// Swap back to original vertices
				std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
			}
		}
	}
}

inline void Renderer::Render_W2_Part2()
{
	//std::vector<Mesh> meshes_world
	//{
	//	Mesh{
	//		{ // Vertices
	//			Vertex{ {-3.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.f,0.f} },
	//			Vertex{ {0.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.f} },
	//			Vertex{ {3.f, 3.f, -2.f}, ColorRGB{colors::White}, {1.f,0.f}},
	//			Vertex{ {-3.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.f,0.5f} },
	//			Vertex{ {0.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.5f} },
	//			Vertex{ {3.f, 0.f, -2.f}, ColorRGB{colors::White}, {1.f,0.5f}},
	//			Vertex{ {-3.f, -3.f, -2.f}, ColorRGB{colors::White}, {0,1.f} },
	//			Vertex{ {0.f, -3.f, -2.f}, ColorRGB{colors::White}, {0.5f,1.f} },
	//			Vertex{ {3.f, -3.f, -2.f} , ColorRGB{colors::White}, {1.f,1.f}},
	//		},

	//		//{ // Indices ( TriangleList )
	//		//	3, 0, 1,	1, 4, 3,   4, 1, 2,
	//		//	2, 5, 4,	6, 3, 4,	4, 7, 6,
	//		//	7, 4, 5,	5, 8, 7
	//		//},
	//		{ // Indices ( TriangleStrip )
	//			3, 0, 4, 1, 5, 2,
	//			2, 6,
	//			6, 3, 7, 4, 8, 5
	//		},

	//		PrimitiveTopology::TriangleStrip
	//	}
	//};


	//// *** PROJECTION STAGE ***
	//// Transform the vector with the World Space vertices to NDC space vertices
	//std::vector<Vertex> vertices_ndc{};
	//VertexTransformationFunction_W2_Part1(meshes_world, vertices_ndc);

	//// *** RASTERIZATION STAGE ***
	//// Convert coordinates from NDC to Screen Space 
	//std::vector<Vertex> vertices_ssv{};
	//vertices_ssv.reserve(vertices_ndc.size());
	//Vertex screenSpaceVertex{};
	//for (const Vertex& vertex : vertices_ndc)
	//{
	//	screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
	//	screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
	//	screenSpaceVertex.position.z = vertex.position.z;
	//	screenSpaceVertex.color = vertex.color;
	//	screenSpaceVertex.uv = vertex.uv;
	//	vertices_ssv.emplace_back(screenSpaceVertex);
	//}

	//std::vector<Uint32> meshes_indices{ meshes_world.at(0).indices };

	//if (meshes_world.at(0).primitiveTopology == PrimitiveTopology::TriangleList)
	//{
	//	// Loop through all triangles ( Every 3 indeces is one triangle )
	//	for (size_t triangleIdx{ 0 }; triangleIdx < meshes_indices.size(); triangleIdx += 3)
	//	{
	//		RenderPixel(vertices_ssv, meshes_indices, triangleIdx);
	//	}
	//}
	//else
	//{
	//	// TriangleStrip Mode
	//	// Loop through all triangles ( Loop shifting varies depending on our PrimitiveTopology mode )
	//	for (size_t triangleIdx{ 0 }; triangleIdx + 2 < meshes_indices.size(); ++triangleIdx)
	//	{
	//		if (triangleIdx % 2 != 0)
	//		{
	//			// Odd triangle -> Swap the last two vertices
	//			std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
	//		}

	//		// If no surface area ( Two identical indeces ) then we are in a degenerate triangle
	//		if (meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 1] ||
	//			meshes_indices[triangleIdx + 1] == meshes_indices[triangleIdx + 2]
	//			|| meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 2])
	//		{
	//			if (triangleIdx % 2 != 0)
	//			{
	//				// Swap back to original
	//				std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
	//			}
	//			// Degenerate triangle -> Go next one
	//			continue;
	//		}

	//		RenderPixel(vertices_ssv, meshes_indices, triangleIdx);

	//		if (triangleIdx % 2 != 0)
	//		{
	//			// Swap back to original vertices
	//			std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
	//		}
	//	}
	//}
}

inline void Renderer::Render_W2_Part3()
{
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{ // Vertices
				Vertex{ {-3.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.f,0.f} },
				Vertex{ {0.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.f} },
				Vertex{ {3.f, 3.f, -2.f}, ColorRGB{colors::White}, {1.f,0.f}},
				Vertex{ {-3.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.f,0.5f} },
				Vertex{ {0.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.5f} },
				Vertex{ {3.f, 0.f, -2.f}, ColorRGB{colors::White}, {1.f,0.5f}},
				Vertex{ {-3.f, -3.f, -2.f}, ColorRGB{colors::White}, {0,1.f} },
				Vertex{ {0.f, -3.f, -2.f}, ColorRGB{colors::White}, {0.5f,1.f} },
				Vertex{ {3.f, -3.f, -2.f} , ColorRGB{colors::White}, {1.f,1.f}},
			},

			//{ // Indices ( TriangleList )
			//	3, 0, 1,	1, 4, 3,   4, 1, 2,
			//	2, 5, 4,	6, 3, 4,	4, 7, 6,
			//	7, 4, 5,	5, 8, 7
			//},
			{ // Indices ( TriangleStrip )
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};


	// *** PROJECTION STAGE ***
	// Transform the vector with the World Space vertices to NDC space vertices
	VertexTransformationFunction_W2(meshes_world);

	// *** RASTERIZATION STAGE ***
	
	for (auto& mesh : meshes_world)
	{
		// Convert coordinates from NDC to Screen/Raster Space 
		std::vector<Vertex> vertices_ssv{};
		vertices_ssv.reserve(mesh.vertices.size());
		Vertex screenSpaceVertex{};
		for (const Vertex& vertex : mesh.vertices)
		{
			screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
			screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
			screenSpaceVertex.position.z = vertex.position.z;
			screenSpaceVertex.color = vertex.color;
			screenSpaceVertex.uv = vertex.uv;
			vertices_ssv.emplace_back(screenSpaceVertex);
		}

		std::vector<Uint32> meshes_indices{ mesh.indices };

		if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
		{
			// Loop through all triangles ( Every 3 indeces is one triangle )
			for (size_t triangleIdx{ 0 }; triangleIdx < meshes_indices.size(); triangleIdx += 3)
			{
				RenderPixel(vertices_ssv, meshes_indices, triangleIdx);
			}
		}
		else
		{
			// TriangleStrip Mode
			// Loop through all triangles ( Loop shifting varies depending on our PrimitiveTopology mode )
			for (size_t triangleIdx{ 0 }; triangleIdx + 2 < meshes_indices.size(); ++triangleIdx)
			{
				if (triangleIdx % 2 != 0)
				{
					// Odd triangle -> Swap the last two vertices
					std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
				}

				// If no surface area ( Two identical indeces ) then we are in a degenerate triangle
				if (meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 1] ||
					meshes_indices[triangleIdx + 1] == meshes_indices[triangleIdx + 2]
					|| meshes_indices[triangleIdx] == meshes_indices[triangleIdx + 2])
				{
					if (triangleIdx % 2 != 0)
					{
						// Swap back to original
						std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
					}
					// Degenerate triangle -> Go next one
					continue;
				}

				RenderPixel(vertices_ssv, meshes_indices, triangleIdx);

				if (triangleIdx % 2 != 0)
				{
					// Swap back to original vertices
					std::swap(meshes_indices[triangleIdx + 1], meshes_indices[triangleIdx + 2]);
				}
			}
		}
	}
	
}


inline void Renderer::Render_W3()
{
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{ // Vertices
				Vertex{ {-3.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.f,0.f} },
				Vertex{ {0.f, 3.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.f} },
				Vertex{ {3.f, 3.f, -2.f}, ColorRGB{colors::White}, {1.f,0.f}},
				Vertex{ {-3.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.f,0.5f} },
				Vertex{ {0.f, 0.f, -2.f}, ColorRGB{colors::White}, {0.5f,0.5f} },
				Vertex{ {3.f, 0.f, -2.f}, ColorRGB{colors::White}, {1.f,0.5f}},
				Vertex{ {-3.f, -3.f, -2.f}, ColorRGB{colors::White}, {0,1.f} },
				Vertex{ {0.f, -3.f, -2.f}, ColorRGB{colors::White}, {0.5f,1.f} },
				Vertex{ {3.f, -3.f, -2.f} , ColorRGB{colors::White}, {1.f,1.f}},
			},

			{ // Indices ( TriangleList )
				3, 0, 1,	1, 4, 3,   4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},
			//{ // Indices ( TriangleStrip )
			//	3, 0, 4, 1, 5, 2,
			//	2, 6,
			//	6, 3, 7, 4, 8, 5
			//},

			PrimitiveTopology::TriangleList
		}
	};


	for (auto& mesh : meshes_world)
	{
		// 1º Move to WorldMatrix
		mesh.UpdateTransforms();
	}

	// PROJECTION STAGE 
	VertexTransformationFunction_W3(meshes_world);

	// *** RASTERIZATION STAGE ***
	for (auto& mesh : meshes_world)
	{
		// To do the conversion from NDC to raster space only once per mesh
		bool isNewMesh{ true };
		std::vector<Vertex_Out> vertices_ssv{};

		// FRUSTRUM CULLING
		// Ignore triangles that are outside the frustum
		for (size_t triangleIdx{ 0 }; triangleIdx < mesh.indices.size(); triangleIdx += 3)
		{
			// Check if any vertex of the triangle is inside the frustum
			bool keepTriangle = true;

			for (int j = 0; j < 3; ++j)
			{
				const Vertex_Out& vertex = mesh.vertices_out[mesh.indices[triangleIdx + j]];
				if (vertex.position.x < -1.f || vertex.position.x > 1.f ||
					vertex.position.y < -1.f || vertex.position.y > 1.f || vertex.position.z < -1.f ||
					vertex.position.z > 1.f)
				{
					// OUTSIDE frustrum -> Ignore triangle
					keepTriangle = false;
					break;
				}
			}

			if (!keepTriangle)
				continue; // At least one vertex is outside frustrum -> Go to next triangle


			// Vertices from this triangle INSIDE THE FRUSTRUM
			if (isNewMesh == true)
			{
				// Convert coordinates from NDC to Screen/Screen Space 
				vertices_ssv.reserve(mesh.vertices_out.size());
				Vertex_Out screenSpaceVertex{};
				for (const Vertex_Out& vertex : mesh.vertices_out)
				{

					screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
					screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
					screenSpaceVertex.position.z = vertex.position.z;
					screenSpaceVertex.position.w = vertex.position.w;
					screenSpaceVertex.color = vertex.color;
					screenSpaceVertex.uv = vertex.uv;
					vertices_ssv.emplace_back(screenSpaceVertex);
				}

				// Dont convert again vertices to raster space for this mesh. Only for a different mesh
				isNewMesh = false;
			}

			RenderPixel_W3(vertices_ssv, mesh.indices, triangleIdx);

		}
	}

}

void Renderer::Render_W3_Part2()
{

	std::vector<Mesh> meshes_world;
	meshes_world.emplace_back(m_TuktukMesh);

	// PROJECTION STAGE 
	VertexTransformationFunction_W3(meshes_world);

	// *** RASTERIZATION STAGE ***
	for (auto& mesh : meshes_world)
	{
		// To do the conversion from NDC to raster space only once per mesh
		bool isNewMesh{ true };
		std::vector<Vertex_Out> vertices_ssv{};

		// FRUSTRUM CULLING
		// Ignore triangles that are outside the frustum
		for (size_t triangleIdx{ 0 }; triangleIdx < mesh.indices.size(); triangleIdx += 3)
		{
			// Check if any vertex of the triangle is inside the frustum
			bool keepTriangle = true;

			for (int j = 0; j < 3; ++j)
			{
				const Vertex_Out& vertex = mesh.vertices_out[mesh.indices[triangleIdx + j]];
				if (vertex.position.x < -1.f || vertex.position.x > 1.f ||
					vertex.position.y < -1.f || vertex.position.y > 1.f || vertex.position.z < -1.f ||
					vertex.position.z > 1.f)
				{
					// OUTSIDE frustrum -> Ignore triangle
					keepTriangle = false;
					break;
				}
			}

			if (!keepTriangle)
				continue; // At least one vertex is outside frustrum -> Go to next triangle


			// Vertices from this triangle INSIDE THE FRUSTRUM
			if (isNewMesh == true)
			{
				// Convert coordinates from NDC to Screen/Screen Space 
				vertices_ssv.reserve(mesh.vertices_out.size());
				Vertex_Out screenSpaceVertex{};
				for (const Vertex_Out& vertex : mesh.vertices_out)
				{

					screenSpaceVertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
					screenSpaceVertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
					screenSpaceVertex.position.z = vertex.position.z;
					screenSpaceVertex.position.w = vertex.position.w;
					screenSpaceVertex.color = vertex.color;
					screenSpaceVertex.uv = vertex.uv;
					screenSpaceVertex.normal = vertex.normal;
					screenSpaceVertex.viewDirection = vertex.viewDirection;
					screenSpaceVertex.tangent = vertex.tangent;
					vertices_ssv.emplace_back(screenSpaceVertex);
				}

				// Dont convert again vertices to raster space for this mesh. Only for a different mesh
				isNewMesh = false;
			}
		
			//RenderPixel_W3(vertices_ssv, mesh.indices, triangleIdx);
			RenderPixel_W4(vertices_ssv, mesh.indices, triangleIdx);

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

void Renderer::VertexTransformationFunction_W2(std::vector<Mesh>& meshes_in) const
{
	Vertex viewSpaceVertex{};
	Vertex projectedVertex{};
	std::vector<Vertex> vertices;
	for (auto& mesh : meshes_in)
	{
		for (const auto& vertex : mesh.vertices)
		{
			// Multiply each vertex with the ViewMatrix -> Transform to View(Space)
			viewSpaceVertex.position = m_Camera.viewMatrix.TransformPoint(vertex.position);

			// Vertices are not in NDC -> Transform them to NDC(Space)
			// ... Apply Perspective Divide to each projected point ( x and y in [-1, 1] range)
			projectedVertex.position.x = viewSpaceVertex.position.x / viewSpaceVertex.position.z;
			projectedVertex.position.y = viewSpaceVertex.position.y / viewSpaceVertex.position.z;
			projectedVertex.position.z = viewSpaceVertex.position.z;

			// ... Mapping x and y to Camera Settings & Screen size
			projectedVertex.position.x = projectedVertex.position.x / (m_Camera.fov * m_AspectRatio);
			projectedVertex.position.y = projectedVertex.position.y / m_Camera.fov;
			projectedVertex.color = vertex.color;
			projectedVertex.uv = vertex.uv;

			// Now our vertex is in NDC(Space)
			//mesh.vertices_out.emplace_back(projectedVertex);
			vertices.emplace_back(projectedVertex);
		}	

		mesh.vertices = vertices;
		vertices.clear();
	}
	
}

void Renderer::VertexTransformationFunction_W3(std::vector<Mesh>& meshes_in) const
{
	Vector4 worldViewProjectionVertex{};
	Vertex_Out vertexNDC{};

	for (auto& mesh : meshes_in)
	{
		// Same matrix for all vertices within one mesh
		Matrix worldViewProjectionMatrix{ mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };
		mesh.vertices_out.reserve(mesh.vertices.size());

		for (const auto& vertex : mesh.vertices)
		{
			Vector4 vertex4{ vertex.position.x, vertex.position.y, vertex.position.z, 1 };
			worldViewProjectionVertex = worldViewProjectionMatrix.TransformPoint(vertex4);

			// PERSPECTIVE DIVIDE
			// Transform vertices to NDC(Space)
			// ... Apply Perspective Divide to each projected point
			vertexNDC.position.x = worldViewProjectionVertex.x / worldViewProjectionVertex.w;
			vertexNDC.position.y = worldViewProjectionVertex.y / worldViewProjectionVertex.w;
			vertexNDC.position.z = worldViewProjectionVertex.z / worldViewProjectionVertex.w;
			vertexNDC.position.w = worldViewProjectionVertex.w;
			vertexNDC.uv = vertex.uv;
			vertexNDC.color = vertex.color;
			vertexNDC.normal = mesh.worldMatrix.TransformVector(vertex.normal);
			vertexNDC.tangent = vertex.tangent;
			vertexNDC.viewDirection = mesh.worldMatrix.TransformPoint(vertex.position) - m_Camera.origin;	// Direction from camera to Vertex

			mesh.vertices_out.emplace_back(vertexNDC);
		}
	}

	
}

inline void Renderer::RenderPixel(const std::vector<dae::Vertex>& vertices_ssv, const std::vector<uint32_t>& meshes_indices,
	size_t triangleIdx) const
{
	Vector2 pixel{};

	// Convert to Vector2
	Vector2 v0{ vertices_ssv[meshes_indices[(triangleIdx)]].position.x, vertices_ssv[meshes_indices[triangleIdx]].position.y };
	Vector2 v1{ vertices_ssv[meshes_indices[(triangleIdx + 1)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 1]].position.y };
	Vector2 v2{ vertices_ssv[meshes_indices[(triangleIdx + 2)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 2]].position.y };

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
	// ... Max are a little big bigger to avoid spots between multiple boundingbox 
	// when they are close to each other ( Black lines )
	minX = minX < 0 ? 0 : minX;
	maxX = maxX > m_Width ? m_Width : maxX + 5;
	minY = minY < 0 ? 0 : minY;
	maxY = maxY > m_Height ? m_Height : maxY + 5;


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

			//// CHECK IF PIXEL INSIDE TRIANGLE 
			////.... Check for every edge from the triangle if the point is on the right side
			Vector2 edge{};
			Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle

			// BAYCENTRIC COORDINATES FOR OPTIMIZATION
			// ... Store the results from the Cross Products (Weights)
			float w0, w1, w2;
			float totalWeight;

			// Weight of V0
			toPoint = pixel - v1;
			edge = v2 - v1;
			w0 = Vector2::Cross(edge, toPoint);
			if (w0 < 0)
				continue;	// Pixel NOT inside triangle -> Go to next pixel

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

			// Pixel INSIDE triangle !
			totalWeight = w0 + w1 + w2;

			// Calculate the final weights by dividing with 
			// the total area of the parallelogram
			w0 = w0 / totalWeight;
			w1 = w1 / totalWeight;
			w2 = w2 / totalWeight;

			// DEPTH TEST 
			// ... Check if pixel is closer than the one in the depth buffer	
			int pixelIndex{ px + (py * m_Width) };

			// Correct interpolated depth
			float v0Z{ (1 / vertices_ssv[meshes_indices[triangleIdx]].position.z) * w0 };
			float v1Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.z) * w1 };
			float v2Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.z) * w2 };

			float interpolatedDepth{ 1 / (v0Z + v1Z + v2Z)};

			// Use the barycentric weights to calcualte the interpolated depth of the pixel inside
			/*float pixelDepth{ (w0 * vertices_ssv[meshes_indices[triangleIdx]].position.z) + (w1 * vertices_ssv[meshes_indices[triangleIdx + 1]].position.z) +
				(w2 * vertices_ssv[meshes_indices[triangleIdx + 2]].position.z) };*/

			if (interpolatedDepth >= m_pDepthBufferPixels[pixelIndex])
				continue;  // Pixel further away. Don't render it

			// This pixel is closer -> Render the color of this one
			// Store it
			m_pDepthBufferPixels[pixelIndex] = interpolatedDepth;

			Vector2 uv0{ (vertices_ssv[meshes_indices[triangleIdx]].uv / vertices_ssv[meshes_indices[triangleIdx]].position.z ) * w0};
			Vector2 uv1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].uv / vertices_ssv[meshes_indices[triangleIdx + 1]].position.z) * w1 };
			Vector2 uv2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].uv / vertices_ssv[meshes_indices[triangleIdx + 2]].position.z) * w2 };

			/*Vector2 uv{ (vertices_ssv[meshes_indices[triangleIdx]].uv * w0) + (vertices_ssv[meshes_indices[triangleIdx + 1]].uv * w1)
				+ (vertices_ssv[meshes_indices[triangleIdx + 2]].uv * w2) };*/

			Vector2 uv{ (uv0 + uv1 + uv2) * interpolatedDepth };
			finalColor = m_pTexture->Sample(uv);

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

inline void Renderer::RenderPixel_W3(const std::vector<dae::Vertex_Out>& vertices_ssv, const std::vector<uint32_t>& meshes_indices,
	size_t triangleIdx) const
{
	Vector2 pixel{};

	// Convert to Vector2
	Vector2 v0{ vertices_ssv[meshes_indices[(triangleIdx)]].position.x, vertices_ssv[meshes_indices[triangleIdx]].position.y };
	Vector2 v1{ vertices_ssv[meshes_indices[(triangleIdx + 1)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 1]].position.y };
	Vector2 v2{ vertices_ssv[meshes_indices[(triangleIdx + 2)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 2]].position.y };

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
	// ... Max are a little big bigger to avoid spots between multiple boundingbox 
	// when they are close to each other ( Black lines )
	minX = minX < 0 ? 0 : minX;
	maxX = maxX > m_Width ? m_Width : maxX + 5;
	minY = minY < 0 ? 0 : minY;
	maxY = maxY > m_Height ? m_Height : maxY + 5;


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

			//// CHECK IF PIXEL INSIDE TRIANGLE 
			////.... Check for every edge from the triangle if the point is on the right side
			Vector2 edge{};
			Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle

			// BAYCENTRIC COORDINATES FOR OPTIMIZATION
			// ... Store the results from the Cross Products (Weights)
			float w0, w1, w2;
			float totalWeight;

			// Weight of V0
			toPoint = pixel - v1;
			edge = v2 - v1;
			w0 = Vector2::Cross(edge, toPoint);
			if (w0 < 0)
				continue;	// Pixel NOT inside triangle -> Go to next pixel

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

			// Pixel INSIDE triangle !
			totalWeight = w0 + w1 + w2;

			// Calculate the final weights by dividing with 
			// the total area of the parallelogram
			w0 = w0 / totalWeight;
			w1 = w1 / totalWeight;
			w2 = w2 / totalWeight;

			// DEPTH TEST 
			// ... Check if pixel is closer than the one in the depth buffer	
			int pixelIndex{ px + (py * m_Width) };

			// Correct interpolated depth ( With Vz )
			float v0Z{ (1 / vertices_ssv[meshes_indices[triangleIdx]].position.z) * w0 };
			float v1Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.z) * w1 };
			float v2Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.z) * w2 };

			float interpolatedDepthZ{ 1 / (v0Z + v1Z + v2Z) };

			// Check if in range [0, 1]
			if (interpolatedDepthZ < 0.f || interpolatedDepthZ > 1.f)
				continue;

			if (interpolatedDepthZ > m_pDepthBufferPixels[pixelIndex])
				continue;  // Pixel further away. Don't render it

			// This pixel is closer -> Render the color of this one
			// Store it
			m_pDepthBufferPixels[pixelIndex] = interpolatedDepthZ;

			if (m_UseDepthBufferColor)
			{
				// Use depth buffer Color
				finalColor = Remap(interpolatedDepthZ, 0.998f, 1.f);
			}
			else
			{
				// Use Texture

				// Use the Vw to interpolate vertex attributes with correct depth interpolation
				// since Vw is storing the the actual z value
				v0Z = (1 / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0;
				v1Z = (1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1;
				v2Z = (1 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2;

				float interpolatedDepthW{ 1 / (v0Z + v1Z + v2Z) };

				Vector2 uv0{ (vertices_ssv[meshes_indices[triangleIdx]].uv / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector2 uv1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].uv / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector2 uv2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].uv / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };


				Vector2 interpolatedUV{ (uv0 + uv1 + uv2) * interpolatedDepthW };
			
				finalColor = m_pTexture->Sample(interpolatedUV);
			}


			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

inline void Renderer::RenderPixel_W4(const std::vector<dae::Vertex_Out>& vertices_ssv, const std::vector<uint32_t>& meshes_indices,
	size_t triangleIdx) const
{
	Vector2 pixel{};

	// Convert to Vector2
	Vector2 v0{ vertices_ssv[meshes_indices[(triangleIdx)]].position.x, vertices_ssv[meshes_indices[triangleIdx]].position.y };
	Vector2 v1{ vertices_ssv[meshes_indices[(triangleIdx + 1)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 1]].position.y };
	Vector2 v2{ vertices_ssv[meshes_indices[(triangleIdx + 2)]].position.x, vertices_ssv[meshes_indices[triangleIdx + 2]].position.y };

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
	// ... Max are a little big bigger to avoid spots between multiple boundingbox 
	// when they are close to each other ( Black lines )
	minX = minX < 0 ? 0 : minX;
	maxX = maxX > m_Width ? m_Width : maxX + 5;
	minY = minY < 0 ? 0 : minY;
	maxY = maxY > m_Height ? m_Height : maxY + 5;


	// Loop only through the pixels that Bounding box cover
	for (int px{ minX }; px < maxX; ++px)
	{
		for (int py{ minY }; py < maxY; ++py)
		{
			// Center pixel
			pixel.x = px + 0.5f;
			pixel.y = py + 0.5f;

			// Black color for pixels outside triangle
			ColorRGB finalColor{ };

			//// CHECK IF PIXEL INSIDE TRIANGLE 
			////.... Check for every edge from the triangle if the point is on the right side
			Vector2 edge{};
			Vector2 toPoint{};		// Used to determine if pixel point in the right side of triangle

			// BAYCENTRIC COORDINATES FOR OPTIMIZATION
			// ... Store the results from the Cross Products (Weights)
			float w0, w1, w2;
			float totalWeight;

			// Weight of V0
			toPoint = pixel - v1;
			edge = v2 - v1;
			w0 = Vector2::Cross(edge, toPoint);
			if (w0 < 0)
				continue;	// Pixel NOT inside triangle -> Go to next pixel

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

			// Pixel INSIDE triangle !
			totalWeight = w0 + w1 + w2;

			// Calculate the final weights by dividing with 
			// the total area of the parallelogram
			w0 = w0 / totalWeight;
			w1 = w1 / totalWeight;
			w2 = w2 / totalWeight;

			// DEPTH TEST 
			// ... Check if pixel is closer than the one in the depth buffer	
			int pixelIndex{ px + (py * m_Width) };

			// Correct interpolated depth ( With vz )
			float v0Z{ (1 / vertices_ssv[meshes_indices[triangleIdx]].position.z) * w0 };
			float v1Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.z) * w1 };
			float v2Z{ (1 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.z) * w2 };

			float interpolatedDepthZ{ 1 / (v0Z + v1Z + v2Z) };

			// Check if in range [0, 1]
			if (interpolatedDepthZ < 0.f || interpolatedDepthZ > 1.f)
				continue;

			if (interpolatedDepthZ > m_pDepthBufferPixels[pixelIndex])
				continue;  // Pixel further away. Don't render it

			// This pixel is closer -> Render the color of this one
			// Store it
			m_pDepthBufferPixels[pixelIndex] = interpolatedDepthZ;

			if (m_UseDepthBufferColor)
			{
				// Use depth buffer Color
				finalColor = Remap(interpolatedDepthZ, 0.998f, 1.f);
			}
			else
			{
				// Use Texture

				// VERTEX ATTRIBUTE INTERPOLATION

				// Use the Vw to interpolate vertex attributes with correct depth interpolation
				// since Vw is storing the the actual z value
				v0Z = (1 / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0;
				v1Z = (1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1;
				v2Z = (1 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2;

				float interpolatedDepthW{ 1 / (v0Z + v1Z + v2Z) };

				// UV 
				Vector2 uv0{ (vertices_ssv[meshes_indices[triangleIdx]].uv / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector2 uv1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].uv / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector2 uv2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].uv / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				Vector2 interpolatedUV{ (uv0 + uv1 + uv2) * interpolatedDepthW };

				// NORMAL
				Vector3 normalV0{ (vertices_ssv[meshes_indices[triangleIdx]].normal / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector3 normalV1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].normal / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector3 normalV2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].normal / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				Vector3 interpolatedNormal{ (normalV0 + normalV1 + normalV2) * interpolatedDepthW };

				// COLOR
				ColorRGB colorV0{ (vertices_ssv[meshes_indices[triangleIdx]].color / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				ColorRGB colorV1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].color / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				ColorRGB colorV2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].color / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				ColorRGB interpolatedColor{ (colorV0 + colorV1 + colorV2) * interpolatedDepthW };

				// TANGENT
				Vector3 tangentV0{ (vertices_ssv[meshes_indices[triangleIdx]].tangent / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector3 tangentV1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].tangent / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector3 tangentV2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].tangent / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				Vector3 interpolatedTangent{ (tangentV0 + tangentV1 + tangentV2) * interpolatedDepthW };

				// VIEW DIRECTION
				Vector3 viewDirV0{ (vertices_ssv[meshes_indices[triangleIdx]].viewDirection / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector3 viewDirV1{ (vertices_ssv[meshes_indices[triangleIdx + 1]].viewDirection / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector3 viewDirV2{ (vertices_ssv[meshes_indices[triangleIdx + 2]].viewDirection / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				Vector3 interpolatedViewDir{ (viewDirV0 + viewDirV1 + viewDirV2) * interpolatedDepthW };

				// POSITION
				Vector2 positionV0{ (v0 / vertices_ssv[meshes_indices[triangleIdx]].position.w) * w0 };
				Vector2 positionV1{ (v1 / vertices_ssv[meshes_indices[triangleIdx + 1]].position.w) * w1 };
				Vector2 positionV2{ (v2 / vertices_ssv[meshes_indices[triangleIdx + 2]].position.w) * w2 };
				Vector2 interpolatedPos{ (positionV0 + positionV1 + positionV2) * interpolatedDepthW };


				// OUTPUT RASTERIZATION STAGE 
				// VERTEX_OUT structure with interpolated attributes
				Vertex_Out v{
					Vector4{ interpolatedPos.x, interpolatedPos.y, interpolatedDepthZ, interpolatedDepthW },
					interpolatedColor,
					interpolatedUV,
					interpolatedNormal,
					interpolatedTangent,
					interpolatedViewDir
				};

				// PIXEL SHADING STAGE
				finalColor = PixelShading(v);

				//finalColor = m_pTexture->Sample(interpolatedUV);
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

inline ColorRGB Renderer::PixelShading(const Vertex_Out& v) const
{
	// Direction of a directional light
	Vector3 lightDirection = { -.577f, .577f, -.577f };

	float lightIntensity = 7.0f;

	// ** LAMBERT'S COSINE LAW ** -> Measure the OBSERVED AREA
	const float viewAngle{ Vector3::Dot(v.normal.Normalized(), lightDirection)};
	if (viewAngle < 0)
		return { 0.f, 0.f, 0.f};  // If it is below 0 the point on the surface points away from the light
				// ( It doesn't contribute for the finalColor)

	return ColorRGB{ viewAngle, viewAngle, viewAngle }; // ObservedArea Only 


}


inline ColorRGB Renderer::Remap(float value, float fromLow, float fromHigh) const
{

	float remapped = (value - fromLow) / (fromHigh - fromLow);

	return ColorRGB{ remapped, remapped, remapped };
}

void Renderer::ToggleFinalColorMode()
{
	m_UseDepthBufferColor = !m_UseDepthBufferColor;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
