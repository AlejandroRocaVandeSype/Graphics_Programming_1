#ifndef DirectX_VEHICLEEFFECT
#define DirectX_VEHICLEEFFECT

#include "Effect.h"

namespace dae 
{
	class VehicleEffect final : public Effect
	{
	public:
		VehicleEffect(ID3D11Device* pDevice, const std::wstring& assetPath);
		virtual ~VehicleEffect();


		void UpdateWorldMatrix(const Matrix& worldMatrix);

		void SetMaps(const std::unordered_map<std::string, Texture*>& textures) override;

		void SetDiffuseMap(const Texture* pDiffuseText);
		void SetSpecularMap(const Texture* pSpecularText);
		void SetGlossinessMap(const Texture* pGlossinessText);
		void SetNormalMap(const Texture* pNormalText);
		void SetCameraVar(const Vector3& cameraPos);

		//ID3DX11EffectTechnique* GetTechnique() const;

		// Toggle Rendering parameters
		void ToggleTechnique();
		void ToggleNormalMap();
		void ToggleShadingMode(const int shadingMode);

	private:

		// Sampler types
		ID3DX11EffectTechnique* m_pLinearTech;
		ID3DX11EffectTechnique* m_pPointTech;
		ID3DX11EffectTechnique* m_pAnisotropicTech;

		// Matrices
		ID3DX11EffectMatrixVariable* m_pWorldVar;

		// Shader Resources
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVar;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVar;

		// Rendering Variables
		ID3DX11EffectScalarVariable* m_pUseNormalMapVar;
		ID3DX11EffectScalarVariable* m_pShadingModeVar;

		ID3DX11EffectVectorVariable* m_pCameraPosVar;

		short m_CurrentTech;
		const short m_NrTechniques;

		void ShaderBinding();

	};
}

#endif


