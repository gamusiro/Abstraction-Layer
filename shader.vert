#version 450

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Normal;
layout (location = 2) in vec4 TexCoord;

uniform mat4 World;
uniform mat4 View;
uniform mat4 Projection;

out vec3 vertColor;

void main()
{
	mat4 wvp = Projection * View * World;

	gl_Position = wvp * Position;
	vertColor	= Normal.xyz;
};