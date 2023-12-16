#ifndef DirectX_TEXTURE
#define DirectX_TEXTURE

namespace dae 
{
	class Texture final
	{
	public:
		~Texture();

		static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);

		ID3D11ShaderResourceView* GetRSV() const;

	private:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);

		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;


	};
}


#endif