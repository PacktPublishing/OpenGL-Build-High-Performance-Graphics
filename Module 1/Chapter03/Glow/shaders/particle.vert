#version 330 core
   
layout(location=0) in vec3 vVertex; //object space vertex position

//uniform
uniform mat4 MVP;	//combined modelview projection matrix
 
//vertex shader output
smooth out vec4 color;	

const vec4 colors[8]=vec4[8](vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1),
							 vec4(1,1,0,1), vec4(0,1,1,1), vec4(1,0,1,1),
							 vec4(0.5,0.5,0.5,1),  vec4(1,1,1,1)) ;
 
void main()
{ 	 	 
	//multiply object space vertex with the combined MVP matrix to get the clip space position
	gl_Position = MVP*vec4(vVertex,1);
	//get the colour of the particle from the colours constant array usign the gl_VertexID as index
	color = colors[gl_VertexID/4]; 
}