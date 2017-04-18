#version 330 core
  
layout(location=0) in vec3 vVertex;		//object space vertex position
layout(location=1) in vec3 vNormal;		//object space normal
 
//uniform
uniform mat4 MVP;	//combined modelview projection

//vertex shader outputs
smooth out vec3 position;	//object space vertex position
smooth out vec3 normal;		//object space normal

void main()
{ 	
	//output the object space position and normal
	position = vVertex;
	normal = vNormal;

	//multiply the combined MVP matrix with the object space position to get the clip space position 
    gl_Position = MVP*vec4(vVertex,1); 
}
 