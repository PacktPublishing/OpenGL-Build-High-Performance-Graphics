#version 330 core
  
layout(location=0) in vec2 vVertex;  //object space vertex position

smooth out vec2 vUV;	//interpolated varying from the vertex shader

void main()
{ 	 
	//get the clipspace position
	gl_Position = vec4(vVertex.xy*2.0-1.0,0,1); 
	//use the object space vertex position as texture cooridnate
	vUV = vVertex; 
}