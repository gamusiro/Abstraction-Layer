/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Object_Cube12.h
*		Detail	:
===================================================================================*/
#pragma once
#include "Object_Interface.h"

class Object_Cube12 : public IObject
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
	static ID3D12Resource*	m_vertexBuffer;
	ID3D12DescriptorHeap*	m_worldBuffer;
};

