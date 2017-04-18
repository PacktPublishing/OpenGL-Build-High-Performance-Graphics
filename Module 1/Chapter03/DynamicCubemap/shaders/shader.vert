#version 330 core
  
layout(location=0) in vec3 vVertex; //object space position

//uniform
uniform mat4 MVP;	//combined modelview projection

void main()
{ 
	//multiply the combined MVP matrix with the object space position to get the clip space position 
	gl_Position = MVP*vec4(vVertex.xyz,1);
}