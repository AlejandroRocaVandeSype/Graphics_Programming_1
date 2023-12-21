#ifndef DirectX_MESH
#define DirectX_MESH

namespace dae
{
	
	struct Vertex_PosCol
	{
		Vector3 position;
		ColorRGB color{ colors::White };
		Vector2 textureUV{};
		Vector3 normal{};
		Vector3 tangent{};
	};
	class Effect;
	class Texture;
	class Mesh final
	{
	public:
		

		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void Update(const Timer* pTimer);

		void SetMatrices(const Matrix& worldViewProjMatrix);
		void ToggleTechnique();

		Matrix GetWorldMatrix() const;
		void SetCameraVar(const Vector3& cameraPos);

		void RotateY(float yaw);
		void Translate(const Vector3& translation);

	private:

		ID3DX11EffectTechnique* m_pTechnique;
		ID3D11InputLayout* m_pInputLayout;
		Effect* m_pEffect;

		ID3D11Buffer* m_pVertexBuffer;				// Buffer with all the vertices from our MESH
		ID3D11Buffer* m_pIndexBuffer;				//	  "		"	"	"   indices	  "	  "    "
		uint32_t m_NumIndices;

		Texture* m_pDiffuseTex;
		Texture* m_pSpecularTex;
		Texture* m_pGlossinessTex;
		Texture* m_pNormalTex;

		Matrix m_WorldMatrix;

		Matrix m_RotationTransform{};
		Matrix m_TranslationTransform{};
		Matrix m_ScaleTransform{};

		void UpdateTransforms();
	};
}

#endif

