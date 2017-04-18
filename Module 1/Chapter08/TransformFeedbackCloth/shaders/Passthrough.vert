#version 330 core
precision highp float;

layout (location=0) in vec4 position_mass;	//position and mass
uniform mat4 MVP;							//combined modelview projection matrix

void main() 
{  
	//get the clipspace position
	gl_Position = MVP*vec4(position_mass.xyz, 1.0);		
}