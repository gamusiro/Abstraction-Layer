/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Graphics_Interface.h
*		Detail	: 
===================================================================================*/
#pragma once
#include "Virtual_Definitions.h"

class IGraphics
{
public:
	virtual bool Init()		= 0;
	virtual void Uninit()	= 0;
	virtual void Clear()	= 0;
	virtual void Present()	= 0;

protected:
	VDevice*	m_device;
	VContext*	m_context;
};
