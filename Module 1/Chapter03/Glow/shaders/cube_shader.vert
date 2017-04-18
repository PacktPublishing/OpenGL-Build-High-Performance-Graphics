#version 330 core
  
layout(location = 0) in vec3 vVertex;  //object space vertex

//uniform
uniform mat4 MVP;  //combined modelview projection matrix

void main()
{ 	 
	//get clipspace position by multiplying the object space vertex position with the combined modelview
	//projection matrix
	gl_Position = MVP*vec4(vVertex.xyz,1); 
}