#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//fragment shader output
smooth in vec4 color;							//input interpolated colour from the vertex shader

void main()
{ 		
	//simply assign the interpolated colour as the fragment output
	vFragColor = color;  	
}