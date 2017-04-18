#version 330 core

layout(location=0) out vec4 vFragColor; //fragment shader output
 
//uniforms
uniform sampler2D textureMap;	//texture map for the mesh
uniform float useDefault;		//if we want to use a default colour

//shader inputs from the vertex shader
smooth in vec4 diffuse;		//the final attenuated diffuse colour   
smooth in vec2 vUVout;		//interpolated texture coordinates

void main()
{   
	//interpolate between the diffuse+texturemap colour and the diffuse colour
	//based on the value of useDefault. If 0, we return the texture map sample
	//colour with the diffuse colour otherwise we only return the diffuse colour
	vFragColor = mix(texture(textureMap, vUVout)*diffuse, diffuse, useDefault);
}