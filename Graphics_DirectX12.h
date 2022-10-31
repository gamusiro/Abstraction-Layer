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
	/// \brief Constructor
	/// 
	/// \return none
	//**************************************************
	GraphicsDirectX12()
		:m_device(),
		m_commandAllocator(),
		m_commandList(),
		m_commandQueue(),
		m_swapChain(),
		m_renderTargetViewHeap(),
		m_backBuffers(),
		m_depthBuffer(),
		m_depthBufferHeap(),
		m_fence(),
		m_fenceValue(),
		m_rootSignature(),
		m_pipelineState(),
		m_viewport(),
		m_scissorRect()
	{}

	//**************************************************
	/// \brief Initialize DirectX12 
	/// 
	/// \return Success is true
	//**************************************************
	bool Init(int width, int height, void* handle) override;

	//**************************************************
	/// \brief Uninitialize DirectX12
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
	/// \param[in] vData	-> model vertex data
	/// \param[in] vDataNum	-> count vertex
	/// \param[in] iData	-> model index data
	/// \param[in] iDataNum	-> count index
	/// 
	/// \return control id
	//**************************************************
	int CreateVertexBufferAndIndexBuffer(
		const structure::Vertex3D* vData,
		size_t vDataNum,
		const unsigned int* iData,
		size_t iDataNum
	) override;

	//**************************************************
	/// \brief Create matrix buffer
	/// 
	/// \return control id
	//**************************************************
	int CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index) override;

	//**************************************************
	/// \brief Set world matrix
	/// 
	/// \param[in] pos -> position of model in 3d space
	/// \param[in] rot -> angle of model 
	/// \param[in] pos -> scale of model
	/// 
	/// \return none
	//**************************************************
	void SetWorldMatrix(
		int id,
		const DirectX::XMFLOAT3 pos,
		const DirectX::XMFLOAT3 rot,
		const DirectX::XMFLOAT3 scl
	) override;

	//**************************************************
	/// \brief Set view matrix
	/// 
	/// \param[in] pos		-> position of camera in 3d space
	/// \param[in] target	-> position of focus in 3d space 
	/// \param[in] up		-> up
	/// 
	/// \return none
	//**************************************************
	void SetViewMatrix(
		int id,
		const DirectX::XMFLOAT3 pos,
		const DirectX::XMFLOAT3 target,
		const DirectX::XMFLOAT3 up
	) override;

	//**************************************************
	/// \brief Set projection matrix
	/// 
	/// \param[in] fov
	/// \param[in] aspect
	/// \param[in] nearZ
	/// \param[in] farZ
	/// 
	/// \return none
	//**************************************************
	void SetProjectionMatrix(
		int id,
		float fov,
		float aspect,
		float nearZ,
		float farZ
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
	UINT						m_fenceValue;
	ID3D12RootSignature*		m_rootSignature;
	ID3D12PipelineState*		m_pipelineState;
	D3D12_VIEWPORT				m_viewport;
	D3D12_RECT					m_scissorRect;

	struct IndexInfo
	{
		ID3D12Resource* buffer;
		size_t			indexNum;
	};

	std::vector<ID3D12Resource*>	m_vertexBuffers;
	std::vector<IndexInfo>			m_indexBuffers;
	std::vector <ID3D12Resource*>	m_constantBuffers;
};

