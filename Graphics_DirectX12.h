/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_DirectX12.h
*		Detail	:
===================================================================================*/
#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "Graphics_Interface.h"

class GraphicsDirectX12 : public IGraphics
{
	public:
	//**************************************************
	/// \brief Initialize DirectX12 
	/// 
	/// \return Success is true
	//**************************************************
	bool Init(int width, int height, void* handle) override;

	//**************************************************
	/// \brief Uninitialize DirectX11
	/// 
	/// \return none
	//**************************************************
	void Uninit() override;

	//**************************************************
	/// \brief Clear screen
	/// 
	/// \return none
	//**************************************************
	void Clear() override;

	//**************************************************
	/// \brief Present buffer
	/// 
	/// \return none
	//**************************************************
	void Present() override;

	//**************************************************
	/// \brief Create buffer
	/// 
	/// \return control id
	//**************************************************
	int CreateVertexBufferAndIndexBuffer(
		const structure::Vertex3D* vData, size_t vDataNum,
		const unsigned int* iData, size_t iDataNum
	) override;

	//**************************************************
	/// \brief Set vertex buffer and index buffer
	/// 
	/// \param[in] control id
	/// 
	/// \return none
	//**************************************************
	void DrawIndex(int index) override;

private:
	//**************************************************
	/// \brief Create device and swapchain
	/// 
	/// \param[in] width	 ->	buffer size width
	/// \param[in] height	 ->	buffer size height
	/// \param[in] hWnd		 ->	window handle
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateDeviceAndSwapChain(
		const int width,
		const int height,
		const HWND hWnd
	);

	//**************************************************
	/// \brief Create render target view
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateRenderTargetView();

	//**************************************************
	/// \brief Create depth buffer
	/// 
	/// \param[in] width	 ->	buffer size width
	/// \param[in] height	 ->	buffer size height
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateDepthBuffer(
		const int width,
		const int height
	);
	//**************************************************
	/// \brief Create fence
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateFence();

	//**************************************************
	/// \brief Create graphics pipeline
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateGraphicsPipeline();

	//**************************************************
	/// \brief Set Resource barrier
	/// 
	/// \param[in] width	 ->	buffer size width
	/// \param[in] height	 ->	buffer size height
	/// \param[in] hWnd		 ->	window handle
	/// 
	/// \return none
	//**************************************************
	void SetResourceBarrier(
		const UINT index,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after
	);

	//**************************************************
	/// \brief Set viewport
	/// 
	/// \return none
	//**************************************************
	void SetViewport(
		const int width,
		const int height
	);

	//**************************************************
	/// \brief Set scissor rect
	/// 
	/// \return none
	//**************************************************
	void SetScissorRect(
		const int width,
		const int height
	);

	static const UINT			k_backBufferNum = 2;
	ID3D12Device*				m_device;
	ID3D12CommandAllocator*		m_commandAllocator;
	ID3D12GraphicsCommandList*	m_commandList;
	ID3D12CommandQueue*			m_commandQueue;
	IDXGISwapChain4*			m_swapChain;
	ID3D12DescriptorHeap*		m_renderTargetViewHeap;
	ID3D12Resource*				m_backBuffers[k_backBufferNum];
	ID3D12Resource*				m_depthBuffer;
	ID3D12DescriptorHeap*		m_depthBufferHeap;
	ID3D12Fence*				m_fence;
	UINT						m_fenceValue = 0;
	ID3D12RootSignature*		m_rootSignature;
	ID3D12PipelineState*		m_pipelineState;
	D3D12_VIEWPORT				m_viewport{};
	D3D12_RECT					m_scissorRect{};

	struct IndexInfo
	{
		ID3D12Resource*			buffer;
		unsigned int			indexNum;
	};

	std::vector<ID3D12Resource*>	m_vertexBuffers;
	std::vector<IndexInfo>			m_indexBuffers;
};

