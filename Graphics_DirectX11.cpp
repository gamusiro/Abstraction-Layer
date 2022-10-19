/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_DirectX11.cpp
*		Detail	:
===================================================================================*/
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "Graphics_DirectX11.h"
using namespace DirectX;

/* Initialize */
bool GraphicsDirectX11::Init(int width, int height, void* handle)
{
	if (!this->CreateDeviceAndSwapChain(width, height, (HWND)handle))
		return false;

	if (!this->CreateRenderTargetView())
		return false;

	if (!this->CreateDepthStencilView(width, height))
		return false;

	if (!this->CreateRasterizerState())
		return false;

	if (!this->CreateBlendState())
		return false;

	if (!this->CreateDepthStencilState())
		return false;

	if (!this->CreateSamplerState())
		return false;

	if (!this->CreateConstantBuffers())
		return false;

	if (!this->CreateShader())
		return false;

	this->SetViewport(width, height);

	return true;	// Success
}

/* Uninitialize */
void GraphicsDirectX11::Uninit()
{
	SAFE_RELEASE(m_projectionMatrix);
	SAFE_RELEASE(m_viewMatrix);
	SAFE_RELEASE(m_modelMatrix);
	SAFE_RELEASE(m_samplerState);
	SAFE_RELEASE(m_depthStencilState);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_rasterizerState);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_context);
	SAFE_RELEASE(m_device);
}

/* Clear screen */
void GraphicsDirectX11::Clear()
{
	float clearColor[4]{ 0.0f, 0.5f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, D3D11_MAX_DEPTH, NULL);

	m_context->IASetInputLayout(m_inputLayout);
	m_context->VSSetShader(m_vertexShader, nullptr, 0);
	m_context->PSSetShader(m_pixelShader, nullptr, 0);
}

/* Present buffer */
void GraphicsDirectX11::Present()
{
	m_swapChain->Present(true, NULL);
}

// Create device and swapchain
bool GraphicsDirectX11::CreateDeviceAndSwapChain(const int width, const int height, const HWND hWnd)
{
	HRESULT ret{};
	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferCount						= 1;
	desc.BufferDesc.Width					= width;
	desc.BufferDesc.Height					= height;
	desc.BufferDesc.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.RefreshRate.Numerator	= 60;
	desc.BufferUsage						= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow						= hWnd;
	desc.SampleDesc.Count					= 1;
	desc.SampleDesc.Quality					= 0;
	desc.Windowed							= TRUE;

	// Create device and swapchain
	ret = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		NULL,
		nullptr,
		NULL,
		D3D11_SDK_VERSION,
		&desc,
		&m_swapChain,
		&m_device,
		nullptr,
		&m_context
	);
	if (FAILED(ret))
		return false;

	return true;	// Success
}

// Create render target view
bool GraphicsDirectX11::CreateRenderTargetView()
{
	HRESULT ret{};
	ID3D11Texture2D* renderTarget{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	ret = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderTarget);
	if (FAILED(ret))
		return false;

	// Create render target view
	ret = m_device->CreateRenderTargetView(renderTarget, nullptr, &m_renderTargetView);
	if (FAILED(ret))
	{
		SAFE_RELEASE(renderTarget);
		return false;
	}

	SAFE_RELEASE(renderTarget);
	return true;	// Success
}

// Create depth stencil view
bool GraphicsDirectX11::CreateDepthStencilView(const int width, const int height)
{
	HRESULT ret{};
	ID3D11Texture2D* depthStencil{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width					= width;
	textureDesc.Height					= height;
	textureDesc.MipLevels				= 1;
	textureDesc.ArraySize				= 1;
	textureDesc.Format					= DXGI_FORMAT::DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc.Count		= 1;
	textureDesc.SampleDesc.Quality		= 0;
	textureDesc.Usage					= D3D11_USAGE::D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags				= D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags			= 0;
	textureDesc.MiscFlags				= 0;
	ret = m_device->CreateTexture2D(&textureDesc, nullptr, &depthStencil);
	if (FAILED(ret))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	viewDesc.Format			= textureDesc.Format;
	viewDesc.ViewDimension	= D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Flags			= 0;

	ret = m_device->CreateDepthStencilView(depthStencil, &viewDesc, &m_depthStencilView);
	if (FAILED(ret))
	{
		SAFE_RELEASE(depthStencil);
		return false;
	}
		

	// Set to render target
	m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	SAFE_RELEASE(depthStencil);

	return true;	// Success
}

// Create rasterizer state
bool GraphicsDirectX11::CreateRasterizerState()
{
	HRESULT ret{};
	D3D11_RASTERIZER_DESC desc{};
	desc.CullMode			= D3D11_CULL_MODE::D3D11_CULL_NONE;
	desc.FillMode			= D3D11_FILL_MODE::D3D11_FILL_SOLID;
	desc.DepthClipEnable	= true;
	desc.MultisampleEnable	= false;
	ret = m_device->CreateRasterizerState(&desc, &m_rasterizerState);
	if (FAILED(ret))
		return false;

	// Set to rasterizer state
	m_context->RSSetState(m_rasterizerState);

	return true;	// Success
}

// Create blend state
bool GraphicsDirectX11::CreateBlendState()
{
	HRESULT ret{};
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable					= false;
	blendDesc.IndependentBlendEnable				= false;
	blendDesc.RenderTarget[0].BlendEnable			= true;
	blendDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;                     // Alpha value from pixel shader. No pre-blend operation
	blendDesc.RenderTarget[0].DestBlend				= D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;                 // Alpha value from pixel shader. Reverse value
	blendDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;                     // Blend option is add
	blendDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND::D3D11_BLEND_ONE;                           // Data source is "white" color. No pre-blend operation
	blendDesc.RenderTarget[0].DestBlendAlpha		= D3D11_BLEND::D3D11_BLEND_ZERO;                          // Data source is "black" color. No pre-blend operation
	blendDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;                     // Blend option is add
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL; // Writing enable

	// Create blend state
	ret = m_device->CreateBlendState(&blendDesc, &m_blendState);
	if (FAILED(ret))
		return false;

	// Set to blend state
	float blendFactor[]{ 0.0f, 0.0f, 0.0f, 0.0f };
	m_context->OMSetBlendState(m_blendState, blendFactor, UINT_MAX);

	return true;	// Success
}

// Create depth stencil state
bool GraphicsDirectX11::CreateDepthStencilState()
{
	HRESULT ret{};
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable		= true;
	depthStencilDesc.DepthWriteMask		= D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
depthStencilDesc.StencilEnable = false;
ret = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
if (FAILED(ret))
return false;

// Set to depth stencil state
m_context->OMSetDepthStencilState(m_depthStencilState, NULL);

return true;	// Success
}

// Create sampler state
bool GraphicsDirectX11::CreateSamplerState()
{
	HRESULT ret{};
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;	// Linear interpolation
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;

	// Create sampler state
	ret = m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(ret))
		return false;

	// Set to sampler state
	m_context->PSSetSamplers(0, 1, &m_samplerState);

	return true;	// Success
}

// Create constant buffers
bool GraphicsDirectX11::CreateConstantBuffers()
{
	HRESULT ret{};
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(XMMATRIX);
	bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	ret = m_device->CreateBuffer(&bufferDesc, nullptr, &m_modelMatrix);
	if (FAILED(ret))
		return false;

	m_device->CreateBuffer(&bufferDesc, nullptr, &m_viewMatrix);
	if (FAILED(ret))
		return false;

	m_device->CreateBuffer(&bufferDesc, nullptr, &m_projectionMatrix);
	if (FAILED(ret))
		return false;

	// Set to constant buffers
	m_context->VSSetConstantBuffers(0, 1, &m_modelMatrix);      // register b0 model matrix
	m_context->VSSetConstantBuffers(1, 1, &m_viewMatrix);       // register b1 view matrix
	m_context->VSSetConstantBuffers(2, 1, &m_projectionMatrix); // register b2 projection matrix 

	return true;	// Success
}

// Create shader
bool GraphicsDirectX11::CreateShader()
{
	HRESULT ret{};
	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	ret = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "vsmain", "vs_4_0", 0, 0, &vsBlob, nullptr);
	if (FAILED(ret))
		return false;

	// Create vertex shader
	ret = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(ret))
	{
		SAFE_RELEASE(vsBlob);
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC elementDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Set input layout
	ret = m_device->CreateInputLayout(
		elementDesc,
		ARRAYSIZE(elementDesc),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&m_inputLayout
	);
	if (FAILED(ret))
	{
		SAFE_RELEASE(vsBlob);
		return false;
	}
	SAFE_RELEASE(vsBlob);

	ret = D3DCompileFromFile(L"shader.hlsl", nullptr, nullptr, "psmain", "ps_4_0", 0, 0, &psBlob, nullptr);
	if (FAILED(ret))
		return false;

	// Create vertex shader
	ret = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(ret))
	{
		SAFE_RELEASE(psBlob);
		return false;
	}
	SAFE_RELEASE(psBlob);
	
	return true;
}

// Set viewport
void GraphicsDirectX11::SetViewport(const int width, const int height)
{
	D3D11_VIEWPORT viewport{};
	viewport.Width		= FLOAT(width);
	viewport.Height		= FLOAT(height);
	viewport.MaxDepth	= D3D11_MAX_DEPTH;
	m_context->RSSetViewports(1, &viewport);
}
