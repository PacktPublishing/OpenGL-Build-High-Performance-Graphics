#version 330 core

layout(location=0) smooth out vec4 vFragColor;	//fragment shader output
uniform vec4 vColor;							//uniform colour to use as fragment colour

void main()
{ 		
	//assign the input varying as the output fragment colour
	vFragColor = vColor;  
}