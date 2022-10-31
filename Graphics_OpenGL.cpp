/*===================================================================================
*	Date : 2022/10/27(Thurs)
*		Author	: Gakuto.S
*		File	: Graphics_OpenGL.cpp
*		Detail	:
===================================================================================*/
#include <fstream>
#include <algorithm>

#include "Graphics_OpenGL.h"
using namespace structure;

/* Initialize */
bool GraphicsOpenGL::Init(int width, int height, void* handle)
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

    if (!this->CreateShader("shader.vert", "shader.frag"))
        return false;

    if (!this->CreateConstantBuffers())
        return false;

    glViewport(0, 0, width, height);

    // Create vertex layout
    glEnableVertexAttribArray(DATA_TYPE::POSITION);
    glEnableVertexAttribArray(DATA_TYPE::NORMAL);
    glEnableVertexAttribArray(DATA_TYPE::TEXCOORD);

    // Enable to alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Eneble to cull mode
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    // Enable to depth buffer
    glEnable(GL_DEPTH_TEST);

    return true;
}

/* Uninitialize */
void GraphicsOpenGL::Uninit()
{
    glDisableVertexAttribArray(DATA_TYPE::TEXCOORD);
    glDisableVertexAttribArray(DATA_TYPE::NORMAL);
    glDisableVertexAttribArray(DATA_TYPE::POSITION);

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(m_context);
    ReleaseDC(m_hWnd, m_device);
}

/* Clear screen */
void GraphicsOpenGL::Clear()
{
    glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* Present buffer */
void GraphicsOpenGL::Present()
{
    SwapBuffers(m_device);
}

/* Create vertex buffer */
int GraphicsOpenGL::CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum)
{
    std::vector<Vertex3D>       vBuffer(vDataNum);
    std::vector<unsigned int>   iBuffer(iDataNum);

    for (size_t v = 0; v < vDataNum; ++v)
        vBuffer[v] = vData[v];

    for (size_t i = 0; i < iDataNum; ++i)
        iBuffer[i] = iData[i];

    m_vertexBuffer.push_back(vBuffer);
    m_indexBuffer.push_back(iBuffer);

    return 0;
}

/* Set world matrix */
void GraphicsOpenGL::SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos3, const DirectX::XMFLOAT3 rot3, const DirectX::XMFLOAT3 scl3)
{
    UNREFERENCED_PARAMETER(id);

    DirectX::XMMATRIX world, trl, rot, scl;
    trl = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos3));
    rot = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rot3));
    scl = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scl3));
    world = scl * rot * trl;

    glUniformMatrix4fv(m_worldMatrix, 1, GL_FALSE, &world.r[0].m128_f32[0]);
}

/* Set view matrix */
void GraphicsOpenGL::SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up)
{
    UNREFERENCED_PARAMETER(id);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
    
    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, &view.r[0].m128_f32[0]);
}

/* Set projection matrix */
void GraphicsOpenGL::SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ)
{
    UNREFERENCED_PARAMETER(id);
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, nearZ, farZ);

    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, &proj.r[0].m128_f32[0]);
}

/* Draw call */
void GraphicsOpenGL::DrawIndex(int id)
{
    glVertexAttribPointer(DATA_TYPE::POSITION, 3, GL_FLOAT, false, sizeof(Vertex3D), &m_vertexBuffer[id][0].Position);
    glVertexAttribPointer(DATA_TYPE::NORMAL,   3, GL_FLOAT, false, sizeof(Vertex3D), &m_vertexBuffer[id][0].Normal);
    glVertexAttribPointer(DATA_TYPE::TEXCOORD, 2, GL_FLOAT, false, sizeof(Vertex3D), &m_vertexBuffer[id][0].TexCoord);

    glDrawElements(GL_TRIANGLES, GLsizei(m_indexBuffer[id].size()), GL_UNSIGNED_INT, m_indexBuffer[id].data());
}

// Create constant buffer
bool GraphicsOpenGL::CreateConstantBuffers()
{
    m_worldMatrix       = glGetUniformLocation(m_programID, "World");
    m_viewMatrix        = glGetUniformLocation(m_programID, "View");
    m_projectionMatrix  = glGetUniformLocation(m_programID, "Projection");
    return true;
}

// Create shader
bool GraphicsOpenGL::CreateShader(const char* vertexFile, const char* pixelFile)
{
    m_programID = glCreateProgram();
    GLuint vertexID, pixelID;
    
    {// Create vertex shader
        std::string buffer;
        std::ifstream ifs(vertexFile, std::ios::binary);
        if (ifs.fail())
            return false;

        ifs.seekg(0, std::ios::end);
        buffer.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&buffer[0], buffer.size());
        ifs.close();

        const char* source = buffer.c_str();
        vertexID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexID, 1, &source, nullptr);
        glCompileShader(vertexID);
        glAttachShader(m_programID, vertexID);
    }

    {// Create pixel shader
        std::string buffer;
        std::ifstream ifs(pixelFile, std::ios::binary);
        if (ifs.fail())
            return false;

        ifs.seekg(0, std::ios::end);
        buffer.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&buffer[0], buffer.size());
        ifs.close();

        const char* source = buffer.c_str();
        pixelID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(pixelID, 1, &source, nullptr);
        glCompileShader(pixelID);
        glAttachShader(m_programID, pixelID);
    }

    glLinkProgram(m_programID);
    glUseProgram(m_programID);

    glDeleteShader(vertexID);
    glDeleteShader(pixelID);

    return true;
}
