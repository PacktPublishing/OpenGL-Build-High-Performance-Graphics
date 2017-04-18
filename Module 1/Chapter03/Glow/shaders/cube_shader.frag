#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

//uniform color value
uniform vec3 vColor;

void main()
{
	//set the given colour value as the fragment colour
	vFragColor = vec4(vColor.xyz,1);
}