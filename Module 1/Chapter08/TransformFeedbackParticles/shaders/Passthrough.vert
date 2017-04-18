#version 330 core
precision highp float;

layout (location=0) in vec4 position;	//object space vertex position 
uniform mat4 MVP;						//combined modelview projection matrix

void main() 
{  
	//get the clip space position
	gl_Position = MVP*vec4(position.xyz, 1.0);		
}