#version 330 core
  
layout(location=0) in vec2 vVertex;  //object space vertex position
smooth out vec2 vUV;	//interpolated texture coordinates output 
						//to fragment shader

void main()
{ 	 
	//get clipspace position from the given object space vertex position
	gl_Position = vec4(vVertex*2-1.0,0,1);

	//store the vertex position as texture coordinates
	vUV = vVertex;
}