#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

void main()
{
	//return solid white colour as fragment shader output
	vFragColor = vec4(1,1,1,1);
}