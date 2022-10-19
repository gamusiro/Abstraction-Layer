/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube11.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Graphics_DirectX11.h"

#include "Object_Cube11.h"
using namespace structure;

ID3D11Buffer* ObjectCube11::m_vertexBuffer;
ID3D11Buffer* ObjectCube11::m_indexBuffer;

const Vertex3D g_sprite[]
{
	{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
};

const unsigned int g_spriteIndex[]
{
	0, 1, 3,
	2, 3, 0
};

/* Initialize */
bool ObjectCube11::Init()
{
	if (!m_vertexBuffer)
	{
		HRESULT ret{};
		ID3D11Device* device = (ID3D11Device*)Application::Graphics()->Device();
		if (!device)
			return false;

		{// Create vertex buffer
			D3D11_BUFFER_DESC bufferDesc{};
			bufferDesc.ByteWidth	= sizeof(Vertex3D) * 4;
			bufferDesc.Usage		= D3D11_USAGE::D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags	= D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA subResource{};
			subResource.pSysMem = g_sprite;
			ret = device->CreateBuffer(&bufferDesc, &subResource, &m_vertexBuffer);
			if (FAILED(ret))
				return false;
		}

		{// Create Index buffer
			D3D11_BUFFER_DESC bufferDesc{};
			bufferDesc.ByteWidth	= sizeof(unsigned int) * 6;
			bufferDesc.Usage		= D3D11_USAGE::D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags	= D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA subResource{};
			subResource.pSysMem = g_spriteIndex;
			ret = device->CreateBuffer(&bufferDesc, &subResource, &m_indexBuffer);
			if (FAILED(ret))
				return false;
		}
	}

	return true;
}

/* Uninitialize */
void ObjectCube11::Uninit()
{
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);
}

/* Update */
void ObjectCube11::Update()
{
}

/* Draw */
void ObjectCube11::Draw()
{
	ID3D11DeviceContext* context = (ID3D11DeviceContext*)Application::Graphics()->Context();

	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexed(6, 0, 0);
}
