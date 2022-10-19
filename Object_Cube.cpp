/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Graphics_DirectX11.h"
#include "Object_Cube11.h"

#include "Object_Cube.h"

/* Constructor */
ObjectCube::ObjectCube()
	:m_cube(nullptr),
	m_position(),
	m_rotate(),
	m_scale()
{
	switch (Application::Get())
	{
	case Application::USING_API_TYPE::DIRECTX_11:
		m_cube = new ObjectCube11();
	default:
		break;
	}
}

/* Destructor */
ObjectCube::~ObjectCube()
{
	delete m_cube;
}

/* Init */
bool ObjectCube::Init()
{
	m_cube->Init();

	return true;
}

/* Uninit */
void ObjectCube::Uninit()
{
	m_cube->Uninit();
}

/* Update */
void ObjectCube::Update()
{
	m_cube->Update();
}

/* Draw */
void ObjectCube::Draw()
{
	m_cube->Draw();
}
