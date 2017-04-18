#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output
	
//uniform
uniform samplerCube cubeMap;	//cubemap texture sampler

//input from the vertex shader
smooth in vec3 uv;	//interpolated 3D texture coordinate

void main()
{
	//return the colour from the cubemap texture 
	vFragColor = texture(cubeMap, uv);
}