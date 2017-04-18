#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

//uniform
uniform vec3 vColor; //constant colour

void main()
{
	//return constant colour as shader output
	vFragColor = vec4(vColor.xyz,1);
}