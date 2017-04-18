#version 330 core

layout(location = 0) out vec4 vFragColor; //fragment shader output
  
void main()
{
	//set the fragment colour as -fragment depth and fragment depth
	//in the red and green channel. This when combined with min/max
	//blending will help in peeling front and back layers simultaneously
	vFragColor.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
}