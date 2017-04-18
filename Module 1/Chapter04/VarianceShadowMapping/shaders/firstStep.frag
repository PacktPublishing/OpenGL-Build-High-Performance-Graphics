#version 330 core

layout(location=0) out vec4 vFragColor;		//fragment shader output

//input from the vertex shader
smooth in vec4 clipSpacePos;	//clip space vertex position

void main()
{
	//do homogeneous division
	vec3 pos = clipSpacePos.xyz/clipSpacePos.w;

	//add some offset to remove the shadow acne
	pos.z += 0.001;	

	//get depth in 0 to 1 range
	float depth = (pos.z +1)*0.5; 

	//store the depth as first moment
	float moment1 = depth;
	
	//store the depth*depth value as second moment
	float moment2 = depth * depth; //this is from the main vsm paper

	//from chap 8 - GPU Gems 3
	//float dx = dFdx(depth);
	//float dy = dFdy(depth); 
	//float moment2 = depth*depth + 0.25*(dx*dx + dy*dy);

	//store the first and second moment in the red and green channel of the output colour
	vFragColor = vec4(moment1,moment2,0,0);
}