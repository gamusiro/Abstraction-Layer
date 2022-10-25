/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_Interface.h
*		Detail	: 
===================================================================================*/
#pragma once
#include <DirectXMath.h>
#include <vector>

namespace structure
{
	struct Vertex3D
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexCoord;
	};
}

//**************************************************
/// \brief These are roots index for send to shader
/// 
/// \return none
//**************************************************
enum CONSTANT_BUFFER_INDEX
{
	WORLD_MATRIX		= 0,	// World buffer index
	VIEW_MATRIX			= 1,	// View buffer index
	PROJECTION_MATRIX	= 2,	// Projection buffer index
	TEXTURE_INDEX		= 3		// Texture buffer index
};


//**************************************************
/// \brief Object release macro
/// 
/// \return none
//**************************************************
#define SAFE_RELEASE(p)\
	if(p)	p->Release();\
	p = nullptr;\

class IGraphics
{
public:
	virtual ~IGraphics() {};
	virtual bool Init(int width, int height, void* handle)	= 0;
	virtual void Uninit()									= 0;
	virtual void Clear()									= 0;
	virtual void Present()									= 0;
	virtual int	 CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum)	= 0;
	virtual int	 CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index) { return 0; }
	virtual void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl) = 0;
	virtual void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up) = 0;
	virtual void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ) = 0;
	virtual void DrawIndex(int id) = 0;
};
