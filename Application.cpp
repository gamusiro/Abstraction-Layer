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

#include "Object_Cube.h"

/* graphics class instance */
IGraphics*                  Application::m_graphics = nullptr;
Application::USING_API_TYPE Application::m_apiType;

ObjectCube g_Cube;


/* Constructor */
Application::Application(const int width, const int height, const void* hInstance, USING_API_TYPE type)
    : WindowDesktop(width, height, (HINSTANCE)hInstance, L"Application", DefMyWndProc)
{
    m_apiType = type;
}

/* Destructor */
Application::~Application()
{
    delete m_graphics;
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
    default:
        break;
    }

    if (!m_graphics) 
        return false;

    if (!m_graphics->Init(m_width, m_height, this->GetHandle())) 
        return false;

    g_Cube.Init();

    return true;
}

/* Uninitialize */
void Application::Uninit()
{
    g_Cube.Uninit();


    if (!m_graphics)
        return;

    m_graphics->Uninit();
}

/* Update */
void Application::Upadte()
{
}

/* Draw */
void Application::Draw()
{
    m_graphics->Clear();

    g_Cube.Draw();

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
