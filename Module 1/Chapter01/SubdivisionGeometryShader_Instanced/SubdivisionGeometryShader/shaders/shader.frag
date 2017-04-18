#version 330 core
 
layout(location=0) out vec4 vFragColor;	//fragment shader output

void main()
{
	//output a constant white colour vec4(1,1,1,1)
   vFragColor = vec4(1,1,1,1);
}