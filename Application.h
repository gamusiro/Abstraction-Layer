/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Application.h
*		Detail	:
===================================================================================*/
#pragma once
#include <vector>

/*  Application class  */
#include "Window_Desktop.h"

class IGraphics;
class ObjectCamera;
class ObjectCube;

class Application : public WindowDesktop
{
public:
	enum class USING_API_TYPE
	{
		DIRECTX_11,
		DIRECTX_12,
		OPENGL,
		VULKAN
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

	//**************************************************
	/// \brief Get api type
	///  
	/// \return api type
	//**************************************************
	static USING_API_TYPE Get();

private:
	static IGraphics*		m_graphics;
	static USING_API_TYPE	m_apiType;

	ObjectCamera*				m_camera;
	std::vector<ObjectCube*>	m_cubes;
};

