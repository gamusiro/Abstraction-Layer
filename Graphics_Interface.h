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
	virtual int	 CreateMatrixBuffer(int registerIndex = 0) { return 0; }
	virtual void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl) = 0;
	virtual void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up) = 0;
	virtual void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ) = 0;
	virtual void DrawIndex(int id) = 0;
};
