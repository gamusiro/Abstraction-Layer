/*===================================================================================
*	Date : 2022/10/27(Thurs)
*		Author	: Gakuto.S
*		File	: Graphics_OpenGL.h
*		Detail	:
===================================================================================*/
#pragma once
#define GLEW_STATIC
#include <Windows.h>
#include <GL/glew.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")

#include "Graphics_Interface.h"
class GraphicsOpenGL : public IGraphics
{
public:
	bool Init(int width, int height, void* handle) override;
	void Uninit() override;
	void Clear() override;
	void Present() override;
	int	 CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum) override;
	int	 CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index) { UNREFERENCED_PARAMETER(index); return 0; }
	void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl) override;
	void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up) override;
	void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ) override;
	void DrawIndex(int id) override;

private:
	bool CreateConstantBuffers();
	bool CreateShader(const char* vertexFile, const char* pixelFile);

private:
	HDC		m_device;
	HGLRC	m_context;
	HWND	m_hWnd;

	enum DATA_TYPE
	{
		VERTEX = 0,
		NORMAL,
		TEXCOORD
	};

	std::vector<std::vector<structure::Vertex3D>>	m_vertexBuffer;
	std::vector<std::vector<unsigned int>>			m_indexBuffer;

	GLuint m_programID;
	GLuint m_worldMatrix;
	GLuint m_viewMatrix;
	GLuint m_projectionMatrix;
};

