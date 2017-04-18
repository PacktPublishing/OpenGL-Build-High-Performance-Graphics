#version 330 core
  
layout(location = 0) in vec3 vVertex; //object space vertex position
  
//uniforms
uniform mat4 MVP;	//combined modelview projection matrix
uniform mat4 S;		//shadow matrix

//outputs to the fragment shader
smooth out vec3 vUV;		//texture coordinates
smooth out vec4 vLightUVW;	//the shadow texture sampling coordinates

void main()
{  
	//the object space vertex position multiplied by
	//the shadow matrix to get the shadow texture lookup
	//coordinates
	vLightUVW = S*vec4(vVertex,1);

	//clip space position
	gl_Position = MVP*vec4(vVertex,1);

	//3D volume sampling texture coordinates
	vUV = vVertex + vec3(0.5);
}