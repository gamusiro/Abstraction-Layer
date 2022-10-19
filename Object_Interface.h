/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Interface.h
*		Detail	:
===================================================================================*/
#pragma once
class IObject
{
public:
	virtual ~IObject() {};
	virtual bool Init()		= 0;
	virtual void Uninit()	= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;
};


