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
		ID3DX11EffectMatrixVariable* GetWorldMatrix() const;

		void SetDiffuseMap(const Texture* pDiffuseText);
		void SetSpecularMap(const Texture* pSpecularText);
		void SetGlossinessMap(const Texture* pGlossinessText);
		void SetNormalMap(const Texture* pNormalText);
		void SetCameraVar(const Vector3& cameraPos);

		void ToggleTechnique();


	private:

		ID3DX11Effect* m_pEffect;		// Manages a set of state objects, resources, and shaders for implementing a rendering effect
		ID3DX11EffectTechnique* m_pLinearTech;
		ID3DX11EffectTechnique* m_pPointTech;
		ID3DX11EffectTechnique* m_pAnisotropicTech;

		ID3DX11EffectMatrixVariable* m_pWorldViewProjVar;
		ID3DX11EffectMatrixVariable* m_pWorldVar;
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVar;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVar;

		ID3DX11EffectVectorVariable* m_pCameraPosVar;

		short m_CurrentTech;
		const short m_NrTechniques;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetPath);

		// Getters to capture variables and/or techniques from the GPU
		ID3DX11EffectTechnique* GetTechnique(const std::string name);
		ID3DX11EffectMatrixVariable* GetMatrix(const std::string name);
		ID3DX11EffectShaderResourceVariable* GetShaderResource(const std::string name);

		void ShaderBinding();
	};
}

#endif