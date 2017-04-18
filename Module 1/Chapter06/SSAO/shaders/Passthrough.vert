#version 330 core
  
layout(location=0) in vec2 vVertex;		//object space vertex position
smooth out vec2 vUV;					//interpolated texture coordinate

void main()
{ 	 
	//get clipspace position from the object space vertex position
	gl_Position = vec4(vVertex*2-1.0,0,1);

	//pass object space vertex position as the texture coordinate
	vUV = vVertex;
}