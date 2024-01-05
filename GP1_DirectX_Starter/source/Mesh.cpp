#include "pch.h"
#include "Mesh.h"
#include "VehicleEffect.h"
#include "Texture.h"

using namespace dae;

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices,
	const std::wstring& shaderPath, const std::unordered_map<std::string, std::string>& texturesPaths)
	: m_pTechnique{ nullptr }, m_pInputLayout{ nullptr }, m_pEffect{ nullptr }, m_pVertexBuffer{ nullptr },
	  m_pIndexBuffer{ nullptr}, m_NumIndices{ static_cast<uint32_t>(indices.size()) },
	m_TranslationTransform{}, m_RotationTransform{}, m_WorldMatrix{}, m_ScaleTransform{}
{
	//m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx");
	m_pEffect = new VehicleEffect(pDevice, shaderPath);
	m_pTechnique = m_pEffect->GetTechnique();

	// CREATE VERTEX LAYOUT
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	// POSITION
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;			  // Nºelem and type R32 bits G32 bits B32 bits ( 3	elements )
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// COLOR
	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;				
	vertexDesc[1].AlignedByteOffset = 12;							// Position takes 12 bytes ( 3 floats x 4 bytesPerFloat)
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// TEXCOORD
	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;				// Only R32 and G32
	vertexDesc[2].AlignedByteOffset = 24;							
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// NORMAL
	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// TANGENT
	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// CREATE INPUT LAYOUT THROUGH THE TECHNIQUE FROM THE EFFECT
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout
	);

	if (FAILED(result))
		return;

	// CREATE VERTEX BUFFER THROUGH THE DEVICE AND DESCRIPTOR
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_PosCol) * static_cast<uint32_t>(vertices.size());		// Amount of vertices * the size of our struct = Bytes needed
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	// CREATE THE INDEX BUFFER
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;		
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;

	// TEXTURES LOAD FOR OUR MESH
	m_Textures.reserve(texturesPaths.size());
	std::string key{};
	std::string texturePath{};
	for (const auto& path : texturesPaths)
	{
		key = path.first;
		texturePath = path.second;
		m_Textures.emplace(key, Texture::LoadFromFile(pDevice, texturePath));
	}
	m_pEffect->SetMaps(m_Textures);

	
}

Mesh::~Mesh()
{
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = nullptr;
	}

	if (m_pTechnique)
	{
		m_pTechnique->Release();
		m_pTechnique = nullptr;
	}

	delete m_pEffect;

	for (auto& pair : m_Textures)
	{
		delete pair.second;	
	}
	m_Textures.clear();
}


void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
	// SET ALL THE CORRECT DATA IN THE DEVICE CONTEXT
	
	// 1. SET Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. SET Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex_PosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::Update(const Timer* pTimer)
{
	RotateY((PI_DIV_2 / 2.f) * pTimer->GetTotal());
	UpdateTransforms();
}

void Mesh::UpdateTransforms()
{
	//Calculate Final Transform 
	//... left-hand system -> SRT ( NOT TRS )
	m_WorldMatrix = m_ScaleTransform * m_RotationTransform * m_TranslationTransform;

}

void Mesh::RotateY(float yaw)
{
	m_RotationTransform = Matrix::CreateRotationY(yaw);
}

void Mesh::Translate(const Vector3& translation)
{
	m_TranslationTransform = Matrix::CreateTranslation(translation);
}

void Mesh::UpdateMatrices(const Matrix& worldViewProjMatrix)
{
	m_pEffect->UpdateWorldViewProjMatrix(worldViewProjMatrix);
	m_pEffect->UpdateWorldMatrix(m_WorldMatrix);
}

Matrix Mesh::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void Mesh::SetCameraVar(const Vector3& cameraPos)
{
	m_pEffect->SetCameraVar(cameraPos);
}

void Mesh::ToggleTechnique()
{
	m_pEffect->ToggleTechnique();
}

void Mesh::ToggleNormalMap()
{
	m_pEffect->ToggleNormalMap();
}

void Mesh::ToggleShadingMode(const int shadingMode)
{
	m_pEffect->ToggleShadingMode(shadingMode);
}