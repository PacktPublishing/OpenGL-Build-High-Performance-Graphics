#version 330 core
  
layout(location=0) in vec2 vVertex;	//object space vertex position		

smooth out vec2 vUV;	//output texture coordinate varying to the 
						//fragment shader

void main()
{ 	 
	//get clipspace position by using the object space vertex position
	gl_Position = vec4(vVertex.xy*2.0-1.0,0,1); 

	//store the object space vertex position as texture coordinates
	vUV = vVertex; 
}