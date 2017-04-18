#version 330 core
  
layout(location=0) in vec3 vVertex;		 //object space vertex position
 
void main()
{    
	//set the object space position 
	gl_Position =  vec4(vVertex, 1);			
}