#ifndef DirectX_RENDERER
#define DirectX_RENDERER

#include "Camera.h"
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		// Toggle Rendering Parameters
		void ToggleFiltering();
		void ToggleNormalMapUse();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh* m_pMesh;

		Camera m_Camera;

		//DIRECTX
		ID3D11Device* m_pDevice;					// Resources
		ID3D11DeviceContext* m_pDeviceContext;		// Rendering pipeline configuration
		IDXGISwapChain* m_pSwapChain;				// One or more surfaces to store rendering data
		ID3D11Texture2D* m_pDepthStencilBuffer;		// 2D Texture 
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;				// Pointer to the back buffer ( Where we are going to render )
		ID3D11RenderTargetView* m_pRenderTargetView;		// render-target subresources 
		HRESULT InitializeDirectX();
		//...

		void InitializeMeshes();
	};
}

#endif
