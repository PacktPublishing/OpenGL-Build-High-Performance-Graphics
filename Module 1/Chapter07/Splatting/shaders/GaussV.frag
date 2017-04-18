#version 330 core
 
layout(location=0)  out vec4 vFragColor;	 //fragment shader output
smooth in vec2 vUV;		//input interpolated texture coordinate

uniform sampler2D textureMap; //the input image to blur

//constant kernel values for Gaussian smoothing
const float kernel[]=float[21] (0.000272337,  0.00089296, 0.002583865, 0.00659813,  0.014869116,
								0.029570767, 0.051898313, 0.080381679, 0.109868729, 0.132526984, 
								0.14107424,  0.132526984, 0.109868729, 0.080381679, 0.051898313, 
								0.029570767, 0.014869116, 0.00659813,  0.002583865, 0.00089296, 0.000272337);
 

 
void main()
{ 
	//get the inverse of texture size
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 20;
	 
	//go through all neighbors and multiply the kernel value with the obtained 
	//colour from the input image
	for(int i=-10;i<=10;i++) {				
		color += kernel[index--]*texture(textureMap, vUV + (vec2(0,i*delta.y)));
	} 

	//return the filtered colour as fragment output
    vFragColor =  color;	
}