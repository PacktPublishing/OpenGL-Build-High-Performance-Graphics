#version 330 core
  
layout(location=0) in vec2 vVertex; //object space vertex position

//output to the fragment shader
smooth out vec2 vUV;			//2D texture coordinates

void main()
{   	
	//get clipspace position from the object space position
	gl_Position = vec4(vVertex.xy*2-1.0,0,1);	 

	//set the object space position as the 2D texture coordinates
	vUV = vVertex;
}