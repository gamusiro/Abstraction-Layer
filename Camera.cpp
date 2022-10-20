/*===================================================================================
*	Date : 2022/10/20(Thurs)
*		Author	: Gakuto.S
*		File	: Camera.cpp
*		Detail	:
===================================================================================*/
#include "Application.h"
#include "Graphics_Interface.h"

#include "Camera.h"
using namespace DirectX;

/* Initialize */
bool ObjectCamera::Init(int width, int height)
{
    m_viewID        = Application::Graphics()->CreateMatrixBuffer();
    m_projectionID  = Application::Graphics()->CreateMatrixBuffer();

    m_fov       = XM_PIDIV2;
    m_aspect    = float(width / height);
    m_nearZ     = 0.1f;
    m_farZ      = 100.0f;

    m_position  = { 0, 5, 5 };
    m_up        = { 0, 1, 0 };

    return true;
}

/* Uninitialize */
void ObjectCamera::Uninit()
{

}

/* Set camera for 3d */
void ObjectCamera::Set3D()
{
    static float rad;
    rad += 0.01f;

    m_position.x = 5 * cosf(rad);
    m_position.z = 5 * sinf(rad);

    Application::Graphics()->SetViewMatrix(
        m_viewID,
        m_position,
        m_target,
        m_up
    );
    
    Application::Graphics()->SetProjectionMatrix(
        m_projectionID,
        m_fov,
        m_aspect,
        m_nearZ,
        m_farZ
    );
}
