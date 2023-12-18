#ifndef DirectX_EFFECT
#define DirectX_EFFECT

namespace dae
{
	class Texture;
	class Effect final
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetPath);
		~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;
		ID3DX11EffectMatrixVariable* GetWorldViewProjMatrix() const;

		void SetDiffuseMap(Texture* pDiffuseTexture);

		void ToggleTechnique();


	private:

		ID3DX11Effect* m_pEffect;		// Manages a set of state objects, resources, and shaders for implementing a rendering effect
		ID3DX11EffectTechnique* m_pLinearTech;
		ID3DX11EffectTechnique* m_pPointTech;
		ID3DX11EffectTechnique* m_pAnisotropicTech;

		ID3DX11EffectMatrixVariable* m_pWorldViewProjVariable;
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

		short m_CurrentTech;
		const short m_NrTechniques;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetPath);
	};
}

#endif