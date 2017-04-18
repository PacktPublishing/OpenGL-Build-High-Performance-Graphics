#version 330 core
  
layout(location = 0) in vec3 vVertex;	//object space vertex position
layout(location = 1) in vec3 vNormal;	//object space vertex normal

//combined modelview projection matrix uniform 
uniform mat4 MVP;  
 
//output object space normal
smooth out vec3 outNormal;

void main()
{  
	//get the clipspace position
	gl_Position = MVP*vec4(vVertex.xyz,1);
	//output object space normal
    outNormal = vNormal;
}