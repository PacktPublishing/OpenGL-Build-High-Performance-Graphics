#version 330 core
  
layout(location=0) in vec3 vVertex;		//object space vertex position

uniform mat4 MVP;	//combined modelview projection

//output from the vertex shader
smooth out vec4 clipSpacePos;	//clip space position

void main()
{ 	 
	//get the clipspace vertex position by multiplying the object space vertex 
	//position with the combined modelview project matrix
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//output the clip space position
	clipSpacePos = gl_Position;
}