#version 330 core
  
layout(location=0) in vec3 vVertex;		//object space vertex position

//output colour from the vertex shader
smooth out vec3 vColor;					//output colour to fragment shader

//uniform 
uniform mat4 MVP;  //combined modelview projection matrix

void main()
{ 	 
	//get the clipspace position
	gl_Position = MVP*vec4(vVertex.xyz,1);

	//get the colour from the object space vertex position by offsetting the vertex position
    vColor = vVertex+0.5;
}