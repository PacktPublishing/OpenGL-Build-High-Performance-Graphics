#version 330 core

layout(location = 0) out vec4 vFragColor; //output fragment colour

uniform vec4 vColor;	//colour uniform

void main()
{
	//just set the vColor uniform as the fragment colour
   vFragColor = vColor;
}