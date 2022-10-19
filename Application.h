/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Application.h
*		Detail	:
===================================================================================*/
#pragma once
#include "Virtual_Definitions.h"	// Virtual define header

/*  Application class  */
#include "Window_Desktop.h"
class Application : public WindowDesktop
{
public:
	//**************************************************
	/// \brief Application class constructor
	/// 
	/// \param[in] width	 ->	window width
	/// \param[in] height	 ->	window height
	/// \param[in] hInstance ->	handle instance for windows desktop app
	/// 
	/// \return none
	//**************************************************
	Application(
		const int	width,
		const int	height,
		const void* hInstance
	);

	//**************************************************
	/// \brief Initialize application
	/// 
	/// \return Success is true
	//**************************************************
	bool Init();
	
	//**************************************************
	/// \brief Uninitialize application
	///  
	/// \return none
	//**************************************************
	void Uninit();

	//**************************************************
	/// \brief Update application
	///  
	/// \return none
	//**************************************************
	void Upadte();

	//**************************************************
	/// \brief Draw in application window
	///  
	/// \return none
	//**************************************************
	void Draw();
};

