#version 330 core

layout(location=0) out vec4 vFragColor;			//fragment shader output

//uniforms
uniform samplerCube cubeMap;	//cubemap texture sampler
uniform vec3 eyePosition;		//eye position in object space

//input from the vertex shader
smooth in vec3 position;		//object space position
smooth in vec3 normal;			//object space normal


void main() { 
	//normalize the normal
	vec3 N = normalize(normal);

	//get the normalized view vector from the object space vertex 
	//position and object space camera position
	vec3 V = normalize(position-eyePosition);

	//reflect the view vector at the normal and use this as a texture colour lookup 
	//from the the cubemap  
	vFragColor = texture(cubeMap, reflect(V,N));	 
}