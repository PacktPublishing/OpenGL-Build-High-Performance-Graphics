#version 330 core
  
layout(location=0) out vec4 vFragColor;  //fragment shader output
 
smooth in vec2 vUV;	//interpolated texture coordinate from vertex shader

//uniforms
uniform sampler2D textureMap; //texture map to display

void main()
{ 	 
	//return the texture sample value at the given texture coordinate
	vFragColor = texture(textureMap, vUV); 
}