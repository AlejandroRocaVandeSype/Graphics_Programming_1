#ifndef DirectX_EFFECT
#define DirectX_EFFECT

#include <unordered_map>

namespace dae
{
	class Matrix;
	class Texture;
	class Effect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetPath);
		virtual ~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		void UpdateWorldViewProjMatrix(const Matrix& worldViewProjMatrix);

		virtual void SetMaps(const std::unordered_map<std::string, Texture*>& textures) = 0;

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;

	private:
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetPath);

	protected:

		ID3DX11Effect* m_pEffect;		// Manages a set of state objects, resources, and shaders for implementing a rendering effect
		ID3DX11EffectTechnique* m_pTechnique;

		// Matrices
		ID3DX11EffectMatrixVariable* m_pWorldViewProjVar;

		// Getters to capture variables and/or techniques from the GPU
		ID3DX11EffectTechnique* GetTechnique(const std::string& name);
		ID3DX11EffectMatrixVariable* GetMatrix(const std::string& name);
		ID3DX11EffectShaderResourceVariable* GetShaderResource(const std::string& name);

	};
}


#endif


