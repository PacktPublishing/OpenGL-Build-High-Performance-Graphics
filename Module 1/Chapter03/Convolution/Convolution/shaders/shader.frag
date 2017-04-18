#version 330 core
 
layout(location=0) out vec4 vFragColor;		//fragment shader output

//input from the vertex shader
smooth in vec2 vUV;							//2D texture coordinates

//uniform
uniform sampler2D textureMap;				//the image to display
 
void main()
{	 
	//get the colour by sampling the given texturemap at the 2D texture coordinates 
    vFragColor = texture(textureMap, vUV); 
}