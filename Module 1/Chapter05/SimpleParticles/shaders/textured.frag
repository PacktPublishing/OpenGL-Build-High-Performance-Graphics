#version 330 core

layout(location=0) out vec4 vFragColor; // fragment shader output

//input from the vertex shader
smooth in vec4 vSmoothColor;	//lienarly interpolated particle colour

uniform sampler2D textureMap;	//particle texture 

void main()
{ 
	//use the particle smooth colour alpha value to fade the colour obtained
	//from the texture lookup 
	vFragColor = texture(textureMap, gl_PointCoord)* vSmoothColor.a;  
}