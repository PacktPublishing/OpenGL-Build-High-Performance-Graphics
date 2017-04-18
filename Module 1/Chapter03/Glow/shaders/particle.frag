#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//vertex shader input colour
smooth in vec4 color;

void main() { 
	//get the offset point coordinates
	vec2 pos = gl_PointCoord-0.5;
	//discard all fragments that are outside the sphere radius
	if(dot(pos,pos)>0.25) 
		discard;
	else
		//for others, return the interpolated colour as the fragment shader output colour
		vFragColor = color;  
}