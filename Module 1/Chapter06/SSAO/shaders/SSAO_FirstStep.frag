#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

smooth in vec3 vEyeSpaceNormal;			//eye space normal from the vertex shader
 
void main()
{ 
	//output the eye space normal as colour, bring it in 0-1 range
	vFragColor = vec4(normalize(vEyeSpaceNormal)*0.5 + 0.5, 1); 
}