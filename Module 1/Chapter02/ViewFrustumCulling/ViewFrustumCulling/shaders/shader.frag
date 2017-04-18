#version 330 core

layout (location = 0) out vec4 vFragColor; //fragment shader output

//uniform
uniform vec4 color;	//constant colour uniform

void main()
{
	//set the given constant colour as the shader output colour
	vFragColor = color;
}