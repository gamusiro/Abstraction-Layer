/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Cube_Vertex11.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Graphics_DirectX11.h"

#include "Cube_Vertex11.h"
using namespace structure;

ID3D11Buffer* CubeVertex11::m_vertexBuffer;
ID3D11Buffer* CubeVertex11::m_indexBuffer;

static const Vertex3D g_planeMeta[]
{
	{{-0.5, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},	// LB
	{{ 0.5, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},	// RB
	{{-0.5,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},	// LT
	{{ 0.5,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},	// RT
};

static const unsigned int g_planeIndex[]
{
	0, 1, 3,
	2, 3, 0
};

/* Load vertex data */
bool CubeVertex11::Load(const wchar_t* fileName)
{
	UNREFERENCED_PARAMETER(fileName);

	HRESULT ret{};
	ID3D11Device* device = (ID3D11Device*)Application::Graphics()->Device();
	if (!device)
		return false;

	// Create vertex buffer
	D3D11_BUFFER_DESC vertexDesc{};
	vertexDesc.ByteWidth	= sizeof(Vertex3D) * 4;
	vertexDesc.Usage		= D3D11_USAGE::D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags	= D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	vertexSubresource.pSysMem = g_planeMeta;
	ret = device->CreateBuffer(&vertexDesc, &vertexSubresource, &m_vertexBuffer);
	if (FAILED(ret))
		return false;

	// Create index buffer
	D3D11_BUFFER_DESC indexDesc{};
	indexDesc.ByteWidth	= sizeof(unsigned int) * 6;
	indexDesc.Usage		= D3D11_USAGE::D3D11_USAGE_DEFAULT;
	indexDesc.BindFlags	= D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexSubresource;
	indexSubresource.pSysMem = g_planeIndex;
	ret = device->CreateBuffer(&indexDesc, &indexSubresource, &m_indexBuffer);
	if (FAILED(ret))
		return false;

	return true;
}

/* Unload vertex buffer */
void CubeVertex11::Unload()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}

/* Set vertex buffer */
void CubeVertex11::Set()
{
	ID3D11DeviceContext* context = (ID3D11DeviceContext*)Application::Graphics()->Context();
	if (!context)
		return;

	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, offset);
}
