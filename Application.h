/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Application.h
*		Detail	:
===================================================================================*/
#pragma once

/*  Application class  */
#include "Window_Desktop.h"

class IGraphics;

class Application : public WindowDesktop
{
public:
	enum class USING_API_TYPE
	{
		DIRECTX_11,
		DIRECTX_12,
		OPENGL,
	};

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
		const void* hInstance,
		USING_API_TYPE type
	);

	//**************************************************
	/// \brief Destructor
	/// 
	/// \return none
	//**************************************************
	~Application();

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

	//**************************************************
	/// \brief Graphics class pointer
	///  
	/// \return pointer of graphics class
	//**************************************************
	static IGraphics* Graphics();

private:
	static IGraphics* m_graphics;
};

