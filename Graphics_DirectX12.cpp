/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_DirectX12.cpp
*		Detail	:
===================================================================================*/
#include <vector>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "Graphics_DirectX12.h"
using namespace structure;
using namespace DirectX;

//**************************************************
/// \brief These are roots index for send to shader
/// 
/// \return none
//**************************************************
enum CONSTANT_BUFFER_INDEX
{
	WORLD_MATRIX		= 0,	// World buffer root index
	VIEW_MATRIX			= 1,	// View buffer root index
	PROJECTION_MATRIX	= 2,	// Projection buffer root index
	TEXTURE_INDEX		= 3		// Texture buffer root index
};

/* Initialize */
bool GraphicsDirectX12::Init(int width, int height, void* handle)
{
	if (!this->CreateDeviceAndSwapChain(width, height, (HWND)handle))
		return false;

	if (!this->CreateRenderTargetView())
		return false;

	if (!this->CreateDepthBuffer(width, height))
		return false;

	if (!this->CreateFence())
		return false;

	if (!this->CreateGraphicsPipeline())
		return false;

	this->SetViewport(width, height);
	this->SetScissorRect(width, height);

	return true;
}

/* Uninitialize */
void GraphicsDirectX12::Uninit()
{
	for (size_t i = 0; i < m_vertexBuffers.size(); ++i)
	{
		SAFE_RELEASE(m_vertexBuffers[i]);
	}

	for (size_t i = 0; i < m_indexBuffers.size(); ++i)
	{
		SAFE_RELEASE(m_indexBuffers[i].buffer);
	}

	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_fence);
	SAFE_RELEASE(m_depthBufferHeap);
	SAFE_RELEASE(m_depthBuffer);
	for (size_t i = 0; i < k_backBufferNum; ++i)
	{
		SAFE_RELEASE(m_backBuffers[i]);
	}
	SAFE_RELEASE(m_renderTargetViewHeap);
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_commandList);
	SAFE_RELEASE(m_commandAllocator);
	SAFE_RELEASE(m_device);
}

/* Clear screen */
void GraphicsDirectX12::Clear()
{
	// Get currently buffer index
	UINT index = m_swapChain->GetCurrentBackBufferIndex();
	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// Set pipeline
	m_commandList->SetPipelineState(m_pipelineState);

	// Set render target
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHeapHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetHeapHandle.ptr += index * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE depthBufferHeapHandle = m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart();
	m_commandList->OMSetRenderTargets(1, &renderTargetHeapHandle, false, &depthBufferHeapHandle);

	float clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(renderTargetHeapHandle, clearColor, 0, nullptr);	// Clear render target view command
	m_commandList->ClearDepthStencilView(													// Clear depth buffer command
		m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH,
		D3D12_MAX_DEPTH,
		0,
		0,
		nullptr
	);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->SetGraphicsRootSignature(m_rootSignature);
}

/* Present buffer */
void GraphicsDirectX12::Present()
{
	// Get currently buffer index
	UINT index = m_swapChain->GetCurrentBackBufferIndex();

	this->SetResourceBarrier(
		index,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT
	);

	// Close command list
	m_commandList->Close();

	// Execute command list
	std::vector<ID3D12CommandList*> commandLists{ m_commandList };
	m_commandQueue->ExecuteCommandLists(commandLists.size(), &commandLists[0]);

	// Wait
	m_commandQueue->Signal(m_fence, ++m_fenceValue);

	if (m_fence->GetCompletedValue() != m_fenceValue)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
		if (fenceEvent)
		{
			m_fence->SetEventOnCompletion(m_fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}
	}

	// Reset
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator, nullptr);

	// Flip
	m_swapChain->Present(1, 0);
}

/* Creats vertex buffer and index buffer */
int GraphicsDirectX12::CreateVertexBufferAndIndexBuffer(
	const structure::Vertex3D* vData, size_t vDataNum,
	const unsigned int* iData, size_t iDataNum
)
{
	int retIndex = m_vertexBuffers.size();

	HRESULT ret{};
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= sizeof(Vertex3D) * vDataNum;
	resourceDesc.Height				= 1;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// Create vertex buffer
	ID3D12Resource* vertexBuffer;
	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&vertexBuffer
	);
	if (FAILED(ret))
		return -1;

	Vertex3D* vertexMap;
	ret = vertexBuffer->Map(0, nullptr, (void**)&vertexMap);
	if (FAILED(ret))
		return -1;

	memcpy(vertexMap, vData, sizeof(Vertex3D) * vDataNum);
	vertexBuffer->Unmap(0, nullptr);
	
	m_vertexBuffers.push_back(vertexBuffer);

	// Create index buffer
	ID3D12Resource* indexBuffer;
	resourceDesc.Width	= sizeof(unsigned int) * iDataNum;
	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&indexBuffer
	);
	if (FAILED(ret))
		return -1;

	unsigned int* indexMap;
	ret = indexBuffer->Map(0, nullptr, (void**)&indexMap);
	if (FAILED(ret))
		return -1;

	memcpy(indexMap, iData, sizeof(unsigned int) * iDataNum);
	indexBuffer->Unmap(0, nullptr);

	m_indexBuffers.push_back({ indexBuffer, iDataNum });

	return retIndex;
}

/* Create descriptor heap for matrix buffer */
int GraphicsDirectX12::CreateMatrixBuffer()
{
	int retIndex = m_constantBuffers.size();

	HRESULT ret{};
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN; 
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;				

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension			= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width				= (sizeof(XMMATRIX) + 0xff) & ~0xff;
	resourceDesc.Height				= 1;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// Create constant buffer
	ID3D12Resource* constantBuffer;
	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&constantBuffer
	);
	if (FAILED(ret))
		return -1;

	m_constantBuffers.push_back(constantBuffer);

	return retIndex;
}

/* Send world matrix to vertex shader */
void GraphicsDirectX12::SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos3, const DirectX::XMFLOAT3 rot3, const DirectX::XMFLOAT3 scl3)
{
	XMMATRIX trl, rot, scl, world;
	trl		= XMMatrixTranslationFromVector(XMLoadFloat3(&pos3));
	rot		= XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rot3));
	scl		= XMMatrixScalingFromVector(XMLoadFloat3(&scl3));
	world	= XMMatrixTranspose(scl * rot * trl);

	XMMATRIX* mat{};
	m_constantBuffers[id]->Map(0, nullptr, (void**)&mat);
	*mat = world;
	m_constantBuffers[id]->Unmap(0, nullptr);

	m_commandList->SetGraphicsRootConstantBufferView(CONSTANT_BUFFER_INDEX::WORLD_MATRIX, m_constantBuffers[id]->GetGPUVirtualAddress());
}

/* Send view matrix to vertex shader */
void GraphicsDirectX12::SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up)
{
	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target), XMLoadFloat3(&up));
	view = XMMatrixTranspose(view);

	XMMATRIX* mat{};
	m_constantBuffers[id]->Map(0, nullptr, (void**)&mat);
	*mat = view;
	m_constantBuffers[id]->Unmap(0, nullptr);

	m_commandList->SetGraphicsRootConstantBufferView(CONSTANT_BUFFER_INDEX::VIEW_MATRIX, m_constantBuffers[id]->GetGPUVirtualAddress());
}

/* Send projection matrix to vertex shader */
void GraphicsDirectX12::SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ)
{
	XMMATRIX projection = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
	projection = XMMatrixTranspose(projection);

	XMMATRIX* mat{};
	m_constantBuffers[id]->Map(0, nullptr, (void**)&mat);
	*mat = projection;
	m_constantBuffers[id]->Unmap(0, nullptr);

	m_commandList->SetGraphicsRootConstantBufferView(CONSTANT_BUFFER_INDEX::PROJECTION_MATRIX, m_constantBuffers[id]->GetGPUVirtualAddress());
}

/* Draw call */
void GraphicsDirectX12::DrawIndex(int index)
{
	D3D12_VERTEX_BUFFER_VIEW vView{};
	vView.BufferLocation	= m_vertexBuffers[index]->GetGPUVirtualAddress();
	vView.SizeInBytes		= UINT(m_vertexBuffers[index]->GetDesc().Width);
	vView.StrideInBytes		= sizeof(Vertex3D);

	D3D12_INDEX_BUFFER_VIEW iView{};
	iView.BufferLocation	= m_indexBuffers[index].buffer->GetGPUVirtualAddress();
	iView.SizeInBytes		= UINT(m_indexBuffers[index].buffer->GetDesc().Width);
	iView.Format			= DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
	
	m_commandList->IASetVertexBuffers(0, 1, &vView);
	m_commandList->IASetIndexBuffer(&iView);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->DrawIndexedInstanced(m_indexBuffers[index].indexNum, 1, 0, 0, 0);
}

// Create device and swapchain
bool GraphicsDirectX12::CreateDeviceAndSwapChain(const int width, const int height, const HWND hWnd)
{
	HRESULT ret{};

	// Create device
	ret = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device),
		(void**)&m_device
	);
	if (FAILED(ret))
		return false;

	// Create command allocator
	ret = m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator),
		(void**)&m_commandAllocator
	);
	if (FAILED(ret))
		return false;

	// Create command list
	ret = m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator,
		nullptr,
		__uuidof(ID3D12GraphicsCommandList),
		(void**)&m_commandList
	);
	if (FAILED(ret))
		return false;

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask	= 0;
	queueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Type		= D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	ret = m_device->CreateCommandQueue(
		&queueDesc,
		__uuidof(ID3D12CommandQueue),
		(void**)&m_commandQueue
	);
	if (FAILED(ret))
		return false;

	// Create factory
	IDXGIFactory6* factory{};
	ret = CreateDXGIFactory2(
		0,
		__uuidof(IDXGIFactory6),
		(void**)&factory
	);
	if (FAILED(ret) || !factory)
		return false;


	// Create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width					= width;
	swapChainDesc.Height				= height;
	swapChainDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo				= false;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.SampleDesc.Quality	= 0;
	swapChainDesc.BufferUsage			= DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount			= k_backBufferNum;
	swapChainDesc.Scaling				= DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode				= DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ret = factory->CreateSwapChainForHwnd(
		m_commandQueue,
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&m_swapChain
	);
	if (FAILED(ret) || !m_swapChain)
	{
		SAFE_RELEASE(factory);
		return false;
	}

	SAFE_RELEASE(factory);
	return true;	// Success
}

// Create render target view
bool GraphicsDirectX12::CreateRenderTargetView()
{
	HRESULT ret{};

	// Create render target view heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = k_backBufferNum;
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ret = m_device->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_renderTargetViewHeap);
	if (FAILED(ret))
		return false;

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	ret = m_swapChain->GetDesc(&swapChainDesc);
	if (FAILED(ret))
		return false;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();	// heap address
	for (UINT i = 0; i < k_backBufferNum; ++i)
	{// Create back buffers
		ret = m_swapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_backBuffers[i]);
		if (FAILED(ret))
			return false;

		m_device->CreateRenderTargetView(m_backBuffers[i], nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return true;	// Success
}

// Create depth buffer
bool GraphicsDirectX12::CreateDepthBuffer(const int width, const int height)
{
	HRESULT ret{};
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width				= width;
	resourceDesc.Height				= height;
	resourceDesc.DepthOrArraySize	= 1;
	resourceDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count	= 1;
	resourceDesc.Flags				= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.MipLevels			= 1;
	resourceDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Alignment			= 0;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;

	// Crear value
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.DepthStencil.Depth	= D3D12_MAX_DEPTH;
	clearValue.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

	ret = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		__uuidof(ID3D12Resource),
		(void**)&m_depthBuffer
	);
	if (FAILED(ret))
		return false;

	// Create heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.NumDescriptors	= 1;
	ret = m_device->CreateDescriptorHeap(
		&descriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap),
		(void**)&m_depthBufferHeap
	);
	if (FAILED(ret))
		return false;

	// Create depth stencil view
	D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	viewDesc.Format			= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	viewDesc.ViewDimension	= D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags			= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthBuffer, &viewDesc, m_depthBufferHeap->GetCPUDescriptorHandleForHeapStart());

	return true;	// Success
}

// Create fence
bool GraphicsDirectX12::CreateFence()
{
	HRESULT ret{};
	ret = m_device->CreateFence(
		m_fenceValue,
		D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE,
		__uuidof(ID3D12Fence),
		(void**)&m_fence
	);
	if (FAILED(ret))
		return false;

	return true;
}

// Create graphics pipeline
bool GraphicsDirectX12::CreateGraphicsPipeline()
{
	HRESULT ret{};
	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	ret = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "vsmain", "vs_4_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(ret))
		return false;

	ret = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "psmain", "ps_4_0", 0, 0, &psBlob, nullptr);
	if (FAILED(ret))
		return false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	graphicsPipeline{};
	graphicsPipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// Definition vertex layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[]
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL",	 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Setting for using shader 
	graphicsPipeline.VS.pShaderBytecode	= vsBlob->GetBufferPointer();
	graphicsPipeline.VS.BytecodeLength	= vsBlob->GetBufferSize();
	graphicsPipeline.PS.pShaderBytecode	= psBlob->GetBufferPointer();
	graphicsPipeline.PS.BytecodeLength	= psBlob->GetBufferSize();

	// Setting for using vertex layout
	graphicsPipeline.InputLayout.pInputElementDescs = inputLayout;
	graphicsPipeline.InputLayout.NumElements		= _countof(inputLayout);

	// Setting for blend state
	graphicsPipeline.BlendState.AlphaToCoverageEnable	= false;
	graphicsPipeline.BlendState.IndependentBlendEnable	= false;

	// Setting for render target view
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};
	renderTargetBlendDesc.BlendEnable			= false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable			= false;
	graphicsPipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	graphicsPipeline.NumRenderTargets			= 1;
	graphicsPipeline.RTVFormats[0]				= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	// Setting for depth stencil state
	graphicsPipeline.DepthStencilState.DepthEnable		= true;
	graphicsPipeline.DepthStencilState.StencilEnable	= false;
	graphicsPipeline.DSVFormat							= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	graphicsPipeline.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
	graphicsPipeline.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;

	// Setting for rasterizer state
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.MultisampleEnable		= false;
	rasterizerDesc.CullMode					= D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode					= D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	rasterizerDesc.DepthClipEnable			= true;
	rasterizerDesc.FrontCounterClockwise	= false;
	rasterizerDesc.DepthBias				= D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp			= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias		= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.AntialiasedLineEnable	= false;
	rasterizerDesc.ForcedSampleCount		= 0;
	rasterizerDesc.ConservativeRaster		= D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	graphicsPipeline.RasterizerState		= rasterizerDesc;

	// Setting to polygon primitive
	graphicsPipeline.IBStripCutValue		= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE::D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphicsPipeline.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// Setting sample
	graphicsPipeline.SampleDesc.Count	= 1;
	graphicsPipeline.SampleDesc.Quality = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	// Setting to texture range
	descriptorRange.NumDescriptors						= 1;
	descriptorRange.RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister					= 0;
	descriptorRange.OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameter[4]{};
	rootParameter[CONSTANT_BUFFER_INDEX::WORLD_MATRIX].ParameterType					= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[CONSTANT_BUFFER_INDEX::WORLD_MATRIX].ShaderVisibility					= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[CONSTANT_BUFFER_INDEX::WORLD_MATRIX].Descriptor.ShaderRegister		= 0;
	rootParameter[CONSTANT_BUFFER_INDEX::WORLD_MATRIX].Descriptor.RegisterSpace			= 0;

	rootParameter[CONSTANT_BUFFER_INDEX::VIEW_MATRIX].ParameterType						= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[CONSTANT_BUFFER_INDEX::VIEW_MATRIX].ShaderVisibility					= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[CONSTANT_BUFFER_INDEX::VIEW_MATRIX].Descriptor.ShaderRegister			= 1;
	rootParameter[CONSTANT_BUFFER_INDEX::VIEW_MATRIX].Descriptor.RegisterSpace			= 0;

	rootParameter[CONSTANT_BUFFER_INDEX::PROJECTION_MATRIX].ParameterType				= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[CONSTANT_BUFFER_INDEX::PROJECTION_MATRIX].ShaderVisibility			= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[CONSTANT_BUFFER_INDEX::PROJECTION_MATRIX].Descriptor.ShaderRegister	= 2;
	rootParameter[CONSTANT_BUFFER_INDEX::PROJECTION_MATRIX].Descriptor.RegisterSpace	= 0;

	rootParameter[CONSTANT_BUFFER_INDEX::TEXTURE_INDEX].ParameterType						= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[CONSTANT_BUFFER_INDEX::TEXTURE_INDEX].DescriptorTable.pDescriptorRanges	= &descriptorRange;
	rootParameter[CONSTANT_BUFFER_INDEX::TEXTURE_INDEX].DescriptorTable.NumDescriptorRanges = 1;
	rootParameter[CONSTANT_BUFFER_INDEX::TEXTURE_INDEX].ShaderVisibility					= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	rootSignatureDesc.pParameters = rootParameter;
	rootSignatureDesc.NumParameters = _countof(rootParameter);

	// Setting for sampler state
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW			= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor			= D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter				= D3D12_FILTER::D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.MaxLOD				= D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD				= 0.0f;
	samplerDesc.ComparisonFunc		= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility	= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;

	// Create root signature
	ID3DBlob* rootSignatureBlob{};
	ret = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSignatureBlob,
		nullptr
	);
	if (FAILED(ret))
		return false;

	// Create root signature
	ret = m_device->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature),
		(void**)&m_rootSignature
	);
	if (FAILED(ret))
	{
		SAFE_RELEASE(rootSignatureBlob);
		return false;
	}

	graphicsPipeline.pRootSignature = m_rootSignature;

	// Create pipeline state
	ret = m_device->CreateGraphicsPipelineState(
		&graphicsPipeline,
		__uuidof(ID3D12PipelineState),
		(void**)&m_pipelineState
	);
	if (FAILED(ret))
	{
		SAFE_RELEASE(rootSignatureBlob);
		return false;
	}

	SAFE_RELEASE(rootSignatureBlob);
	SAFE_RELEASE(psBlob);
	SAFE_RELEASE(vsBlob);

	return true;	// Success
}

// Resource barrier setting
void GraphicsDirectX12::SetResourceBarrier(const UINT index, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrierDesc{};
	barrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource	= m_backBuffers[index];
	barrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore	= before;
	barrierDesc.Transition.StateAfter	= after;
	m_commandList->ResourceBarrier(1, &barrierDesc);
}

// Set viewport
void GraphicsDirectX12::SetViewport(const int width, const int height)
{
	m_viewport.Width	= FLOAT(width);
	m_viewport.Height	= FLOAT(height);
	m_viewport.MaxDepth = D3D12_MAX_DEPTH;
}

// Set scissor rect
void GraphicsDirectX12::SetScissorRect(const int width, const int height)
{
	m_scissorRect.left		= 0;
	m_scissorRect.top		= 0;
	m_scissorRect.right		= m_scissorRect.left + width;
	m_scissorRect.bottom	= m_scissorRect.top + height;
}