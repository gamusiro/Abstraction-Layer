/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube11.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Graphics_Interface.h"

#include "Object_Cube.h"
using namespace structure;

const Vertex3D g_sprite[]
{
	{{-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{{-1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	{{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
};

const unsigned int g_spriteIndex[]
{
	1, 2, 0,
	3, 6, 2,
	7, 4, 6,
	5, 0, 4,
	6, 0, 2,
	3, 5, 7,
	1, 3, 2,
	3, 7, 6,
	7, 5, 4,
	5, 1, 0,
	6, 4, 0,
	3, 1, 5,
};

// vertex and index id
int ObjectCube::m_id = -1;

/* Initialize */
bool ObjectCube::Init()
{
	if (m_id < 0)
	{
		m_id = Application::Graphics()->CreateVertexBufferAndIndexBuffer(
			g_sprite, _countof(g_sprite),
			g_spriteIndex, _countof(g_spriteIndex)
		);
		if (m_id < 0)
			return false;
	}

	m_worldID = Application::Graphics()->CreateMatrixBuffer();

	return true;
}

/* Uninitialize */
void ObjectCube::Uninit()
{
	m_id = -1;
}

/* Update */
void ObjectCube::Update()
{
}

/* Draw */
void ObjectCube::Draw()
{
	Application::Graphics()->SetWorldMatrix(m_worldID, m_position, m_rotate, m_scale);
	Application::Graphics()->DrawIndex(m_id);
}