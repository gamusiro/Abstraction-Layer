/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Application.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Window_Desktop_Procedure.h"

#include "Graphics_DirectX11.h"
#include "Graphics_DirectX12.h"
#include "Graphics_Vulkan.h"

#include "Camera.h"
#include "Object_Cube.h"

/* graphics class instance */
IGraphics*                  Application::m_graphics = nullptr;
Application::USING_API_TYPE Application::m_apiType;


/* Constructor */
Application::Application(const int width, const int height, const void* hInstance, USING_API_TYPE type)
    : WindowDesktop(width, height, (HINSTANCE)hInstance, L"Application", DefMyWndProc)
{
    m_apiType = type;
}

/* Destructor */
Application::~Application()
{
}


/* Initialize */
bool Application::Init()
{
    switch (m_apiType)
    {
    case Application::USING_API_TYPE::DIRECTX_11:
        m_graphics = new GraphicsDirectX11();
        break;
    case Application::USING_API_TYPE::DIRECTX_12:
        m_graphics = new GraphicsDirectX12();
        break;
    case Application::USING_API_TYPE::OPENGL:
        break;
    case Application::USING_API_TYPE::VULKAN:
        m_graphics = new GraphicsVulkan();
        break;
    default:
        return false;
    }

    if (!m_graphics) 
        return false;

    if (!m_graphics->Init(m_width, m_height, this->GetHandle())) 
        return false;
    
    m_camera = new ObjectCamera();
    m_camera->Init(m_width, m_height);

    m_cubes.resize(5);
    for (size_t i = 0; i < m_cubes.size(); ++i)
    {
        m_cubes[i] = new ObjectCube();
        m_cubes[i]->Init();
        m_cubes[i]->SetPosition({ -4.0f + 3.0f * float(i), 0, 0 });
    }

    return true;
}

/* Uninitialize */
void Application::Uninit()
{
    for (size_t i = 0; i < m_cubes.size(); ++i)
    {
        m_cubes[i]->Uninit();
        delete m_cubes[i];
    }

    m_camera->Uninit();
    delete m_camera;

    if (!m_graphics)
        return;

    m_graphics->Uninit();
    delete m_graphics;
}

/* Update */
void Application::Upadte()
{
}

/* Draw */
void Application::Draw()
{
    m_graphics->Clear();
    m_camera->Set3D();

    for (size_t i = 0; i < m_cubes.size(); ++i)
    {
        m_cubes[i]->Draw();
    }

    m_graphics->Present();
}

/* Get graphics class pointer */
IGraphics* Application::Graphics()
{
    return m_graphics;
}

/* Get using api type */
Application::USING_API_TYPE Application::Get()
{
    return m_apiType;
}
