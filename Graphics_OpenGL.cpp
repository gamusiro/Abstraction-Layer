/*===================================================================================
*	Date : 2022/10/27(Thurs)
*		Author	: Gakuto.S
*		File	: Graphics_OpenGL.cpp
*		Detail	:
===================================================================================*/
#include "Graphics_OpenGL.h"

/* Initialize */
bool Graphics_OpenGL::Init(int width, int height, void* handle)
{
    PIXELFORMATDESCRIPTOR formatDescriptor{};
    formatDescriptor.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    formatDescriptor.nVersion    = 1;
    formatDescriptor.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    formatDescriptor.iPixelType  = PFD_TYPE_RGBA;
    formatDescriptor.cColorBits  = 24;
    formatDescriptor.cDepthBits  = 32;
    formatDescriptor.iLayerType  = PFD_MAIN_PLANE;

    // Create device context
    m_hWnd = HWND(handle);
    m_device = GetDC(m_hWnd);
    if (!m_device)
        return false;

    int pixelFormat = ChoosePixelFormat(m_device, &formatDescriptor);
    SetPixelFormat(m_device, pixelFormat, &formatDescriptor);

    // Create gl context
    m_context = wglCreateContext(m_device);
    if (!m_context)
        return false;

    wglMakeCurrent(m_device, m_context);

    if (glewInit() != GLEW_OK)
        return false;

    return true;
}

/* Uninitialize */
void Graphics_OpenGL::Uninit()
{
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(m_context);
    ReleaseDC(m_hWnd, m_device);
}

/* Clear screen */
void Graphics_OpenGL::Clear()
{
    glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* Present buffer */
void Graphics_OpenGL::Present()
{
    SwapBuffers(m_device);
}

/* Create vertex buffer */
int Graphics_OpenGL::CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum)
{
    return 0;
}

/* Set world matrix */
void Graphics_OpenGL::SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl)
{
}

void Graphics_OpenGL::SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up)
{
}

void Graphics_OpenGL::SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ)
{
}

/* Draw call */
void Graphics_OpenGL::DrawIndex(int id)
{
    // glDrawElements(GL_TRIANGLES, );
}

/* Create constant buffer */
bool Graphics_OpenGL::CreateConstantBuffers()
{

    return true;
}
