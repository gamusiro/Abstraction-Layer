/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Interface.h
*		Detail	:
===================================================================================*/
#pragma once
#include <DirectXMath.h>

class ObjectBase {};

class IObject : public ObjectBase
{
public:
	IObject()
		:m_worldID(),
		m_position(),
		m_rotate(),
		m_scale({1, 1, 1})
	{}
	virtual ~IObject() {};
	virtual bool Init()		= 0;
	virtual void Uninit()	= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

	void SetPosition(DirectX::XMFLOAT3 pos) { m_position = pos; }
	void SetRotate(DirectX::XMFLOAT3 rot) { m_position = rot; }
	void SetScale(DirectX::XMFLOAT3 scl) { m_position = scl; }

protected:
	int					m_worldID;
	DirectX::XMFLOAT3	m_position;
	DirectX::XMFLOAT3	m_rotate;
	DirectX::XMFLOAT3	m_scale;
};


