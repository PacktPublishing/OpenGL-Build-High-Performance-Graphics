#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//input from the vertex shader
smooth in vec3 vColor;			//interpolated colour from the fragment shader

void main()
{
	//set the interpolated colour from the vertex shader as the fragment colour
	vFragColor = vec4(vColor.xyz,1);
}