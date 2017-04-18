#version 330 core

layout(location = 0) out vec4 vFragColor; //fragment shader output

smooth in vec3 outNormal;	//varying input from the vertex shader 
							//interpolated by rasterizer
 
void main()
{
	//output the object space normal as colour
	vFragColor = vec4(outNormal,1);
}