#version 330 core
  
layout(location=0) in vec2 vVertex;		//object space vertex position

//vertex shader output to fragment shader
smooth out vec2 vUV;					//texture coordinates

void main()
{ 	 
	//get the clipspace position from the given vertex positions
	gl_Position = vec4(vVertex*2-1.0,0,1);
	//assign the given vertex position to output texture coordinates
	vUV = vVertex;
}