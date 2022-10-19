/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube11.h
*		Detail	:
===================================================================================*/
#pragma once
#include "Graphics_DirectX11.h"
#include "Object_Interface.h"

class ObjectCube11 : public IObject
{
public:
	//**************************************************
	/// \brief Initialize cube use directX11
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
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
};

