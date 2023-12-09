#ifndef DirectX_EFFECT
#define DirectX_EFFECT

namespace dae
{
	class Effect final
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetPath);
		~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		ID3DX11Effect* GetEffect();
		ID3DX11EffectTechnique* GetTechnique();

	private:

		ID3DX11Effect* m_pEffect;		// Manages a set of state objects, resources, and shaders for implementing a rendering effect
		ID3DX11EffectTechnique* m_pTechnique;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetPath);
	};
}

#endif