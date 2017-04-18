#version 330 core
  
layout(location = 0) in vec3 vVertex;	//object space vertex position

//uniform
uniform mat4 MVP;  //combined modelview projection matrix

void main()
{  
	//get the clipspace position
	gl_Position = MVP*vec4(vVertex.xyz,1);
}