#version 330 core
precision highp float;

#extension EXT_gpu_shader4 : require

layout (location =0) in vec4 position_mass; //xyz -> position, w -> mass

//uniforms 
uniform mat4 MV;			//modelview matrix
uniform mat4 MVP;			//combined modelview projection matrix
uniform float pointSize;	//point size in screen space
uniform int selected_index;	//index of selected vertex
uniform in vec4 vColor;		//color uniform
smooth out vec4 oColor;		//output varying to the fragment shader

void main() 
{  	
	vec4 vert = vec4(position_mass.xyz,1);
	//get eye space position
	vec3 pos_eye = (MV * vert).xyz;
	//calculate the point size 
    gl_PointSize = max(1.0, pointSize / (1.0 - pos_eye.z));
	//calculate the clipspace position
	gl_Position = MVP*vec4(position_mass.xyz, 1.0);		
	//if vertex is selected, give it an inverse colour 
	if(selected_index == gl_VertexID)
	   oColor = 1-vColor;
	else
	   oColor = vColor;
}