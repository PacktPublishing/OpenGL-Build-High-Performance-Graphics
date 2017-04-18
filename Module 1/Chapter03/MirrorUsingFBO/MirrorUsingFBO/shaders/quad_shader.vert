#version 330 core
  
layout(location=0) in vec3 vVertex;	//object space vertex position

//vertex shader output
smooth out vec2 vUV;	//output texture coordinate to fragment shader
 
//uniform
uniform mat4 MVP;		//combined modelview projection matrix

void main()
{ 	 
	//multiply the vertex position with the combined modelview projection matrix
	gl_Position = MVP * vec4(vVertex.xyz,1); 

	//obtain the texture coordinate from the object space vertex position
	vUV = vec2( (vVertex.x+1), vVertex.y) *0.5; 
}