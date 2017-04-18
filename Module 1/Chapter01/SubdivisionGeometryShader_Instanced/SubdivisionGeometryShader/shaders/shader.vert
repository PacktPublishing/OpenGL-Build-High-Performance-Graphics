#version 330 core
  
layout(location=0) in vec3 vVertex;  //object space vertex position

uniform mat4 M[4];	//modeling matrix for each instance

void main()
{    
	//get the world space position of each instance vertex
	gl_Position =  M[gl_InstanceID]*vec4(vVertex, 1);		
}