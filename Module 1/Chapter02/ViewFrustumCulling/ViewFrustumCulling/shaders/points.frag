#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

void main()
{
	//if fragment coordinates are less than the sphere radius,
	//we discard the fragment.
	vec2 pos = (gl_PointCoord.xy-0.5);
	if(0.25<dot(pos,pos))
		discard;

	//otherwise, we return a constant fragment colour (blue)
	vFragColor = vec4(0,0,1,1);
}