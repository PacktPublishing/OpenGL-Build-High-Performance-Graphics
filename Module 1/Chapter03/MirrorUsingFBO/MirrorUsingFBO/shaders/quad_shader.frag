#version 330 core

layout(location=0) out vec4 vFragColor;		//output fragment colour

//input from the vertex shader
smooth in vec2 vUV;				//interpolated 2D texture coordinate

//uniform
uniform sampler2D textureMap;	//texture map to display 

void main()
{ 
	//output the colour by lookup in the texturemap using the input texture coordinate
	vFragColor = texture(textureMap, vUV);
}