#version 330 core 
  
layout(location = 0) in vec2 vVertex; //object space vertex position
 
void main()
{  
	//get the clip space position from the object space position
	gl_Position = vec4(vVertex.xy*2 - 1.0,0,1);
}