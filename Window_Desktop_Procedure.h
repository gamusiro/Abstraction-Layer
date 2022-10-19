/*===================================================================================
*	Date : 2022/10/13(Thurs)
*		Author	: Gakuto.S
*		File	: Window_Desctop_Procedure.h
*		Detail	: This header file is window procedure definition file.
===================================================================================*/
#pragma once
#include <Windows.h>

#define DefMyWndProc DestroyWindowThenEndProc
static LRESULT CALLBACK DestroyWindowThenEndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
