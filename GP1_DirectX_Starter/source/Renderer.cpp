#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Utils.h"
#include "VehicleEffect.h"
#include "FireEffect.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow), m_pDevice { nullptr }, m_pDeviceContext{ nullptr },
		m_pSwapChain{ nullptr }, m_pDepthStencilBuffer{ nullptr }, m_pDepthStencilView{ nullptr },
		m_pRenderTargetBuffer{ nullptr }, m_pRenderTargetView{ nullptr }, m_pVehicle{ nullptr }, m_Camera{}
		, m_ShadingMode{ ShadingMode::Combined }
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		float aspectRatio{ m_Width / static_cast<float>(m_Height) };

		// Init Camera
		m_Camera.Initialize(aspectRatio, 45.f, Vector3{ 0.f, 0.f, -50.f });

		InitializeMeshes();

	}

	void Renderer::InitializeMeshes()
	{
		// Create some data for our mesh (NDC coord)
		 // TRIANGLE in World space coord
		/*std::vector<Vertex_PosCol> vertices{
			{ { 0.f, 3.f, 2.f}, {1.f, 0.f, 0.f}},
			{ { 3.f, -3.f, 2.f}, {0.f, 0.f, 1.f}},
			{ { -3.f, -3.f, 2.f}, {0.f, 1.f, 0.f}},
		};

		std::vector<uint32_t> indices{ 0, 1, 2 };*/

		//// QUAD in WSC
		//std::vector<Vertex_PosCol> vertices
		//{
		//	{ {-3.f, 3.f, 2.f}, {1.f, 1.f, 1.f} ,{0.f, 0.f} },    // Top Left  ( {0.f, 0.f} for UV)
		//	{ {3.f, 3.f, 2.f}, {1.f, 1.f, 1.f}, {1.f, 0.f}},     // Top Right
		//	{ {-3.f, -3.f, 2.f}, {1.f, 1.f, 1.f},{0.f, 1.f} },   // Bot Left
		//	{ {3.f, -3.f, 2.f}, {1.f, 1.f, 1.f} , {1.f, 1.f} }      // Bot Right
		//};
		//std::vector<uint32_t> indices{ 0, 1, 2, 1, 3, 2 };		// Two triangles
		//m_pMesh = new Mesh(m_pDevice, vertices, indices);

		// Vehicle Mesh
		std::vector<Vertex_PosCol> vertices_vehicle{};
		std::vector<uint32_t> indices_vehicle{};
		Mesh_Type type = Mesh_Type::vehicle;
		Utils::ParseOBJ("Resources/vehicle.obj", vertices_vehicle, indices_vehicle);

		std::unordered_map<std::string, std::string> vehicleTextures;
		vehicleTextures.emplace("diffuse", "Resources/vehicle_diffuse.png");
		vehicleTextures.emplace("glossiness", "Resources/vehicle_gloss.png");
		vehicleTextures.emplace("normal", "Resources/vehicle_normal.png");
		vehicleTextures.emplace("specular", "Resources/vehicle_specular.png");
		
		m_pVehicle = new Mesh(type, m_pDevice, vertices_vehicle, indices_vehicle, L"Resources/vehicle.fx", vehicleTextures);

		// FireFX Mesh
		std::vector<Vertex_PosCol> vertices_fire{};
		std::vector<uint32_t> indices_fire{};
		Utils::ParseOBJ("Resources/fireFX.obj", vertices_fire, indices_fire);

		Mesh_Type type2 = Mesh_Type::fire;
		std::unordered_map<std::string, std::string> fireTextures;
		fireTextures.emplace("diffuse", "Resources/fireFX_diffuse.png");
		m_pFireFX = new Mesh(type2, m_pDevice, vertices_fire, indices_fire, L"Resources/fire.fx", fireTextures);
	}

	Renderer::~Renderer()
	{
		delete m_pVehicle;
		delete m_pFireFX;

		// RESOURCES ARE RELEASED IN REVERSE ORDER
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
			m_pRenderTargetBuffer = nullptr;
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
			m_pDepthStencilView = nullptr;
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
			m_pDepthStencilBuffer = nullptr;
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
			m_pDeviceContext = nullptr;
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = nullptr;
		}


	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		if (m_DoRotation)
		{
			m_pVehicle->Update(pTimer);
			m_pFireFX->Update(pTimer);
		}
			
	}


	void Renderer::Render() const
	{
		if (!m_IsInitialized)		// Only if DirectX has been init we continue
			return;

		// 1. CLEAR BUFFERS ( RTV & DSV)
		constexpr float color[4] = { 0.39f, 0.59f, 0.93f, 1.f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);	// Back buffer
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);	// Depth buffer

		// Update WorldViewProj matrix before rendering
		Matrix worldViewProjMatrix{ m_pVehicle->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix() };
		m_pVehicle->UpdateMatrices(worldViewProjMatrix);

		worldViewProjMatrix = m_pFireFX->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
		m_pFireFX->UpdateMatrices(worldViewProjMatrix);
		
		//2. SET PIPELINE + INVOKE DRAW CALLS (=RENDER)
		// Render our meshes
		m_pVehicle->Render(m_pDeviceContext);
		
		if(m_DoRenderFireFX)
			m_pFireFX->Render(m_pDeviceContext);


		//3. PRESENT BACKBUFFER (SWAP)
		// After rendering, you present the frame to the screen
		m_pSwapChain->Present(0, 0);

	}


	// TOGGLE RENDERING FUNCTIONS ( ONLY FOR VEHICLE MESH )
	void Renderer::ToggleFiltering()
	{
		VehicleEffect* pVehicleEffect{ dynamic_cast<VehicleEffect*>(m_pVehicle->GetEffect()) };
		if(pVehicleEffect)
			pVehicleEffect->ToggleTechnique();
	}

	void Renderer::ToggleNormalMapUse()
	{

		VehicleEffect* pVehicleEffect{ dynamic_cast<VehicleEffect*>(m_pVehicle->GetEffect()) };
		if (pVehicleEffect)
			pVehicleEffect->ToggleNormalMap();
	}

	void Renderer::ToggleFireFXMesh()
	{
		m_DoRenderFireFX = !m_DoRenderFireFX;
	}

	void Renderer::ToggleRotation(Timer* pTimer)
	{
		if (m_DoRotation)
		{
			
			pTimer->Stop();

		}
		else
		{
			// It was stopped already -> Continue the timer
			pTimer->Start();
		}

		m_DoRotation = !m_DoRotation;
		pTimer->Update();
	}

	void Renderer::ToggleShadingMode()
	{
		switch (m_ShadingMode)
		{
		case dae::Renderer::ShadingMode::ObservedArea:
			std::cout << "Shading Mode : DiffuseLambert " << std::endl;
			m_ShadingMode = ShadingMode::Diffuse;
			break;
		case dae::Renderer::ShadingMode::Diffuse:
			std::cout << "Shading Mode : SpecularPhong" << std::endl;
			m_ShadingMode = ShadingMode::Specular;
			break;
		case dae::Renderer::ShadingMode::Specular:
			std::cout << "Shading Mode : Combined" << std::endl;
			m_ShadingMode = ShadingMode::Combined;
			break;
		case dae::Renderer::ShadingMode::Combined:
			std::cout << "Shading Mode : Cosine Lambert" << std::endl;
			m_ShadingMode = ShadingMode::ObservedArea;
			break;
		}

		VehicleEffect* pVehicleEffect{ dynamic_cast<VehicleEffect*>(m_pVehicle->GetEffect()) };
		if (pVehicleEffect)
			pVehicleEffect->ToggleShadingMode(static_cast<int>(m_ShadingMode));

	}

	HRESULT Renderer::InitializeDirectX()
	{
		// 1. CREATE DEVICE & DEVICE CONTEXT
		D3D_FEATURE_LEVEL feature_level{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };		// For extra layers

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;			// OR operation. 
																// The created device will support the debug layer
																		
#endif

		HRESULT result{ D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &feature_level,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext) };
		if (FAILED(result))
			return result;

		// Create DXGI Factory -> To create DXGI objects
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
		{
			pDxgiFactory->Release();
			return result;
		}
			

		//2. CREATE THE SWAP CHAIN
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the HANDLE (HWND) from the SDL Backbuffer
		// To control the window
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. CREATE DEPTHSTENCIL (DS) & DEPTHSTENCILVIEW (DSV) RESOURCE
		D3D11_TEXTURE2D_DESC depthStencilDesc{};		// Depth buffer with stencil buffer
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;					// Amount of mipMaps we are using
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// FIRST VIEW RESOURCE
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. CREATE RENDER TARGET & RENDERTARGETVIEW

		// Resource
		// Get Buffer allow us to access one of the swap-chain's back buffers.
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		// SECOND VIEW RESOURCE
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5. bIND RTV & DSV views to Output Merge Stage
		// Bind them as the active buffers
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set the ViewPort 
		// Defines where the content of the back buffer will be rendered on the screen
		// DirectX use this viewPort to transform NDC to matching Screen Space Coord
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);		// Only 1 viewPort

		if (pDxgiFactory)
		{
			pDxgiFactory->Release();
			pDxgiFactory = nullptr;
		}
			

		return result;
	}
}
