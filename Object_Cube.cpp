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
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
};

const unsigned int g_spriteIndex[]
{
	0, 1, 3,
	2, 3, 0
};

int g_id = -1;

/* Initialize */
bool ObjectCube::Init()
{
	if (g_id)
	{
		g_id = Application::Graphics()->CreateVertexBufferAndIndexBuffer(
			g_sprite, _countof(g_sprite),
			g_spriteIndex, _countof(g_spriteIndex)
		);
		if (g_id < 0)
			return false;
	}

	return true;
}

/* Uninitialize */
void ObjectCube::Uninit()
{
	g_id = -1;
}

/* Update */
void ObjectCube::Update()
{
}

/* Draw */
void ObjectCube::Draw()
{
	Application::Graphics()->DrawIndex(g_id);
}