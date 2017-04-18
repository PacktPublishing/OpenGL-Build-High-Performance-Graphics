#version 330 core
 
layout(location = 0) in vec3 vVertex;	//vertex position
layout(location = 1) in vec3 vNormal;	//vertex normal
layout(location = 2) in vec2 vUV;		//vertex uv coordinates
 
//uniforms for projection, modelview and normal matrices
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;

//shader outputs to the fragment shader
smooth out vec2 vUVout;					//texture coordinates
smooth out vec3 vEyeSpaceNormal;    	//eye space normals
smooth out vec3 vEyeSpacePosition;		//eye space positions

void main()
{
	//output the texture coordinates
	vUVout=vUV; 
	
	//multiply the object space vertex position with the modelview matrix 
	//to get the eye space position  
	vEyeSpacePosition = (MV*vec4(vVertex,1)).xyz; 

	//multiply the object space normal with the normal matrix to get 
	//the eye space normal
	vEyeSpaceNormal   = N*vNormal;

	//multiply the projection matrix with the eye space position to get
	//the clipspace postion
	gl_Position = P*vec4(vEyeSpacePosition,1);
}