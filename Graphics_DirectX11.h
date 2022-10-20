/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_DirectX11.h
*		Detail	: 
===================================================================================*/
#pragma once
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "Graphics_Interface.h"

class GraphicsDirectX11 : public IGraphics
{
public:
	//**************************************************
	/// \brief Initialize DirectX11 
	/// 
	/// \return Success is true
	//**************************************************
	bool Init(int width, int height, void* handle)	override;

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
	/// \brief Set view matrix
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
	void DrawIndex(int id) override;

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
	/// \brief Create depth stencil view
	///
	/// \param[in] width	 ->	buffer size width
	/// \param[in] height	 ->	buffer size height
	///   
	/// \return Succcess is true
	//**************************************************
	bool CreateDepthStencilView(
		const int width,
		const int height
	);

	//**************************************************
	/// \brief Create rasterizer state
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateRasterizerState();

	//**************************************************
	/// \brief Create blend state
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateBlendState();

	//**************************************************
	/// \brief Create depth stencil state
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateDepthStencilState();

	//**************************************************
	/// \brief Create sampler state
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateSamplerState();

	//**************************************************
	/// \brief Create constant buffers
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateConstantBuffers();

	//**************************************************
	/// \brief Create shader
	///    
	/// \return Succcess is true
	//**************************************************
	bool CreateShader();

	//**************************************************
	/// \brief Create depth stencil view
	///
	/// \param[in] width	 ->	window width
	/// \param[in] height	 ->	window height
	///   
	/// \return none
	//**************************************************
	void SetViewport(
		const int width,
		const int height
	);

private:
	ID3D11Device*				m_device;				// Device Interface
	ID3D11DeviceContext*		m_context;				// DeviceContext Interface
	IDXGISwapChain*				m_swapChain;			// SwapChain Interface
	ID3D11RenderTargetView*		m_renderTargetView; 	// RenderTargetView Interface
	ID3D11DepthStencilView*		m_depthStencilView; 	// DepthStencilView Interface
	ID3D11RasterizerState*		m_rasterizerState;  	// RasterizerState Interface
	ID3D11BlendState*			m_blendState;       	// BlendState Interface
	ID3D11DepthStencilState*	m_depthStencilState;	// DepthStencilState Interface
	ID3D11SamplerState*			m_samplerState;     	// SamplerState Interface
	ID3D11Buffer*				m_modelMatrix;      	// The buffer is model matrix send to vertex buffer 
	ID3D11Buffer*				m_viewMatrix;       	// The buffer is view matrix send to vertex buffer
	ID3D11Buffer*				m_projectionMatrix; 	// The buffer is projection matrix send to vertex buffer
	ID3D11InputLayout*			m_inputLayout;			// Vertex layout Interface
	ID3D11VertexShader*			m_vertexShader;			// Vertex shader Interface
	ID3D11PixelShader*			m_pixelShader;			// Pixel shader Interface

	struct IndexInfo
	{
		ID3D11Buffer* buffer;
		unsigned int indexNum;
	};

	std::vector<ID3D11Buffer*>	m_vertexBuffers;
	std::vector<IndexInfo>		m_indexBuffers;

};