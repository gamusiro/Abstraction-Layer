/*===================================================================================
*	Date : 2022/10/20(Thurs)
*		Author	: Gakuto.S
*		File	: Camera.h
*		Detail	:
===================================================================================*/
#pragma once
#include "Object_Interface.h"

class ObjectCamera : public ObjectBase
{
public:
	bool Init(int width, int height);
	void Uninit();
	void Set3D();

private:
	DirectX::XMFLOAT3	m_position;
	DirectX::XMFLOAT3	m_target;
	DirectX::XMFLOAT3	m_up;
	float				m_aspect;
	float				m_fov;
	float				m_nearZ;
	float				m_farZ;
	int					m_viewID;
	int					m_projectionID;
};

