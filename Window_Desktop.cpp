/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Window_Desktop.cpp
*		Detail	:
===================================================================================*/
#include "Window_Desktop.h"

/* Constructor */ 
WindowDesktop::WindowDesktop(
    const int width, const int height, const HINSTANCE hInstance, LPCWSTR caption, WNDPROC wndProc)
    :WindowInterface(width, height),
    m_hInstance(hInstance),
    m_windowHandle(nullptr),
    m_className(caption)
{
    WNDCLASSEX wcex{};
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_CLASSDC;
    wcex.lpfnWndProc    = wndProc;
    wcex.hInstance      = hInstance;
    wcex.lpszClassName  = caption;
    RegisterClassEx(&wcex);

    m_windowHandle = CreateWindow(
        m_className,
        caption,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width, height,
        nullptr,
        nullptr,
        m_hInstance,
        nullptr
    );

    // Show up the window
    ShowWindow(m_windowHandle, true);
    UpdateWindow(m_windowHandle);
}

/* Destructor */
WindowDesktop::~WindowDesktop()
{
    UnregisterClass(m_className, m_hInstance);
}

/* Close */
bool WindowDesktop::Close()
{
    MSG msg;
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return true;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return false;
}

/* Get HWND */
void* WindowDesktop::GetHandle()
{
    return m_windowHandle;
}
