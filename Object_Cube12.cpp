/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube12.cpp
*		Detail	:
===================================================================================*/
#include <algorithm>
#include "Application.h"

#include "Object_Cube12.h"
using namespace structure;
using namespace DirectX;

const Vertex3D g_sprite[]
{
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
};

const unsigned int g_spriteIndex[]
{
	0, 1, 3,
	2, 3, 0
};

/* Initialize */
bool ObjectCube12::Init()
{
	HRESULT ret{};
	ID3D12Device* device = (ID3D12Device*)Application::Graphics()->Device();

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= sizeof(g_sprite);
	resourceDesc.Height				= 1;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ret = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_vertexBuffer
	);
	if (FAILED(ret))
		return false;

	resourceDesc.Width	= sizeof(g_spriteIndex);
	ret = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_indexBuffer
	);
	if (FAILED(ret))
		return false;

	// Create constant buffer
	resourceDesc.Width = (sizeof(XMMATRIX) + 0xff) & ~0xff;
	ret = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_worldBuffer
	);
	if (FAILED(ret))
		return false;

	Vertex3D* vertexMap;
	ret = m_vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	if (FAILED(ret))
		return false;
	
	std::copy(std::begin(g_sprite), std::end(g_sprite), vertexMap);
	m_vertexBuffer->Unmap(0, nullptr);

	unsigned int* indexMap;
	ret = m_indexBuffer->Map(0, nullptr, (void**)&indexMap);
	if (FAILED(ret))
		return false;

	std::copy(std::begin(g_spriteIndex), std::end(g_spriteIndex), indexMap);
	m_indexBuffer->Unmap(0, nullptr);

	return true;
}

/* Uninitialize */
void ObjectCube12::Uninit()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}

/* Update */
void ObjectCube12::Update()
{
}

/* Draw */
void ObjectCube12::Draw()
{
	ID3D12GraphicsCommandList* context = (ID3D12GraphicsCommandList*)Application::Graphics()->Context();

	D3D12_VERTEX_BUFFER_VIEW bufferView{};
	bufferView.BufferLocation	= m_vertexBuffer->GetGPUVirtualAddress();
	bufferView.SizeInBytes		= sizeof(g_sprite);
	bufferView.StrideInBytes	= sizeof(Vertex3D);

	D3D12_INDEX_BUFFER_VIEW indexView{};
	indexView.BufferLocation	= m_indexBuffer->GetGPUVirtualAddress();
	indexView.Format			= DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	indexView.SizeInBytes		= sizeof(g_spriteIndex);

	context->IASetVertexBuffers(0, 1, &bufferView);
	context->IASetIndexBuffer(&indexView);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
