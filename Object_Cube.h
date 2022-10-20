/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube.h
*		Detail	:
===================================================================================*/
#pragma once
#include "Object_Interface.h"

class ObjectCube : public IObject
{
public:
	//**************************************************
	/// \brief Initialize cube
	/// 
	/// \return Success is true
	//**************************************************
	bool Init()		override;
	
	//**************************************************
	/// \brief Uninitialize cube
	/// 
	/// \return none
	//**************************************************
	void Uninit()	override;
	
	//**************************************************
	/// \brief Update cube
	/// 
	/// \return none
	//**************************************************
	void Update()	override;

	//**************************************************
	/// \brief Draw cube
	/// 
	/// \return none
	//**************************************************
	void Draw()		override;

private:
	static int			m_id;
};

