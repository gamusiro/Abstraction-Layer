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
	//**************************************************
	/// \brief Constructor
	/// 
	/// \return none
	//**************************************************
	GraphicsOpenGL()
		:m_device(),
		m_context(),
		m_hWnd(),
		m_programID(),
		m_worldMatrix(),
		m_viewMatrix(),
		m_projectionMatrix()
	{}

	//**************************************************
	/// \brief Initialize OpenGL
	/// 
	/// \return Success is true
	//**************************************************
	bool Init(int width, int height, void* handle) override;
	
	//**************************************************
	/// \brief Uninitialize OpenGL
	/// 
	/// \return none
	//**************************************************
	void Uninit() override;

	//**************************************************
	/// \brief Clear screen
	/// 
	/// \return none
	//**************************************************
	void Clear() override;

	//**************************************************
	/// \brief Present buffer
	/// 
	/// \return none
	//**************************************************
	void Present() override;
	
	//**************************************************
	/// \brief Create buffer
	/// 
	/// \param[in] vData	-> model vertex data
	/// \param[in] vDataNum	-> count vertex
	/// \param[in] iData	-> model index data
	/// \param[in] iDataNum	-> count index
	/// 
	/// \return control id
	//**************************************************
	int	 CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum) override;
	
	//**************************************************
	/// \brief Create matrix buffer
	/// 
	/// \return control id
	//**************************************************
	int	 CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index) { UNREFERENCED_PARAMETER(index); return 0; }

	//**************************************************
	/// \brief Set world matrix
	/// 
	/// \param[in] pos -> position of model in 3d space
	/// \param[in] rot -> angle of model 
	/// \param[in] pos -> scale of model
	/// 
	/// \return none
	//**************************************************
	void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl) override;
	
	//**************************************************
	/// \brief Set view matrix
	/// 
	/// \param[in] pos		-> position of camera in 3d space
	/// \param[in] target	-> position of focus in 3d space 
	/// \param[in] up		-> up
	/// 
	/// \return none
	//**************************************************
	void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up) override;
	
	//**************************************************
	/// \brief Set projection matrix
	/// 
	/// \param[in] fov
	/// \param[in] aspect
	/// \param[in] nearZ
	/// \param[in] farZ
	/// 
	/// \return none
	//**************************************************
	void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ) override;

	//**************************************************
	/// \brief Set vertex buffer and index buffer
	/// 
	/// \param[in] control id
	/// 
	/// \return none
	//**************************************************
	void DrawIndex(int id) override;

private:
	//**************************************************
	/// \brief Create constant buffer
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateConstantBuffers();
	
	//**************************************************
	/// \brief Create shader
	/// 
	/// \param[in] vertexFile
	/// \param[in] pixelFile
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateShader(const char* vertexFile, const char* pixelFile);

private:
	HDC		m_device;	// Windows device context
	HGLRC	m_context;	// GL context
	HWND	m_hWnd;		// window handle

	enum DATA_TYPE
	{
		POSITION = 0,	// Vertex Position
		NORMAL,			// Vertex Normal
		TEXCOORD		// Vertex Texcoord
	};

	std::vector<std::vector<structure::Vertex3D>>	m_vertexBuffer;
	std::vector<std::vector<unsigned int>>			m_indexBuffer;

	GLuint m_programID;			// Program ID
	GLuint m_worldMatrix;		// World matrix buffer ID for vertex shader 
	GLuint m_viewMatrix;		// View matrix buffer ID for vertex shader 
	GLuint m_projectionMatrix;	// Projection matrix buffer ID for vertex shader
};

