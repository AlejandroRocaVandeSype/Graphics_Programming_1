#ifndef DirectX_MESH
#define DirectX_MESH

namespace dae
{
	
	struct Vertex_PosCol
	{
		Vector3 position;
		ColorRGB color;
	};
	class Effect;
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

	private:

		ID3DX11EffectTechnique* m_pTechnique;
		ID3D11InputLayout* m_pInputLayout;
		Effect* m_pEffect;

		ID3D11Buffer* m_pVertexBuffer;				// Buffer with all the vertices from our MESH
		ID3D11Buffer* m_pIndexBuffer;				//	  "		"	"	"   indices	  "	  "    "
		uint32_t m_NumIndices;
	};
}

#endif

