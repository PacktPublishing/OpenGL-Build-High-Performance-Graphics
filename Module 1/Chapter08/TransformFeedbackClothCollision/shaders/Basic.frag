#version 330 core

layout (location=0) smooth out vec4 vFragColor;		//fragment shader output
smooth in vec4 oColor;								//colour varying from the vertex shader

void main()
{ 		
	//The equation is sqrt(dot(gl_PointCoord-0.5,gl_PointCoord-0.5))>0.5	
	//square both sides gives this.
	//We discard fragments outsize the sphere
	if(dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)	
		discard;
	else
		vFragColor = oColor;   
}