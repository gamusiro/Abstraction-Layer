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
	virtual bool	Init(int width, int height, void* handle)	= 0;
	virtual void	Uninit()									= 0;
	virtual void	Clear()										= 0;
	virtual void	Present()									= 0;
	virtual int		CreateVertexBufferAndIndexBuffer(
		const structure::Vertex3D* vData, size_t vDataSize,
		const unsigned int* iData, size_t iDataSize)			= 0;
	virtual void	DrawIndex(int index)						= 0;
};
