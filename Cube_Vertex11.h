/*===================================================================================
*	Date : 2022/10/19(Wednes)
*		Author	: Gakuto.S
*		File	: Cube_Vertex11.h
*		Detail	:
===================================================================================*/
#pragma once

class ICube {};

class CubeVertex11 : public ICube
{
public:
	bool Load(const wchar_t* fileName);
	void Unload() ;
	void Set() ;

private:
	static ID3D11Buffer*	m_vertexBuffer;
	static ID3D11Buffer*	m_indexBuffer;
};

