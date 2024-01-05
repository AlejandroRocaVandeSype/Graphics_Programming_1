#ifndef DIRECTX_FIREEFFECT
#define DIRECTX_FIREEFFECT

#include "Effect.h"

namespace dae
{
	class FireEffect final : public Effect
	{
	public:
		FireEffect(ID3D11Device* pDevice, const std::wstring& assetPath);
		virtual ~FireEffect();

		void SetMaps(const std::unordered_map<std::string, Texture*>& textures) override;

	private:

		// Shader Resources
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar;
	};
}


#endif // !DIRECTX_FIREEFFECT





