#version 330 core
 
layout (location=0) out vec4 vFragColor;	//fragment shader output

//vertex shader input
smooth in vec2 vUV;			//intepolated 2D texture coordinate

//uniform
uniform sampler2D textureMap;	//texture map

void main()
{	
   	vec4 color = vec4(0);
	//determine the inverse of texture size
	vec2 delta = 1.0/textureSize(textureMap,0);

	//loop through the neighborhood
	for(int j=-3;j<=3;j++) {
		for(int i=-3;i<=3;i++) {	
			//sum all samples in the neighborhodd
			color += texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	//divide by the total number of samples
	color/=49.0;
	//return the average color
    vFragColor =  color;
}