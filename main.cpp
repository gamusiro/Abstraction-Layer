/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: main.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"

/* main */
int __stdcall WinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPSTR lpCmdLine,
	_In_		int nCmdShow
)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	{
		Application app(1280, 780, hInstance, Application::USING_API_TYPE::DIRECTX_11);

		if (app.Init())
		{
			while (!app.Close())
			{
				app.Upadte();
				app.Draw();
			}
		}

		app.Uninit();
	}

	return 0;
}