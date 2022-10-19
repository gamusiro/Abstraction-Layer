/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Application.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"

#include "Window_Desktop.h"
#include "Window_Desktop_Procedure.h"

/* Constructor */
Application::Application(const int width, const int height, const void* hInstance)
    : WindowDesktop(width, height, (HINSTANCE)hInstance, L"Application", DefMyWndProc)
{
}


/* Initialize */
bool Application::Init()
{

    return true;
}

/* Uninitialize */
void Application::Uninit()
{

}

/* Update */
void Application::Upadte()
{
}

/* Draw */
void Application::Draw()
{

}
