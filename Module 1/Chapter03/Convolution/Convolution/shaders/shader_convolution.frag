#version 330 core
 
layout(location=0) out vec4 vFragColor;	//fragment shader output

//input from the vertex shader
smooth in vec2 vUV;						//2D texture coordinates

//shader uniform
uniform sampler2D textureMap;			//the filtering image


//3x3 sharpening kernel 
const float kernel[]=float[9] (-1,-1,-1,
							   -1,8,-1,
							   -1,-1,-1);
 

//3x3 unweighted smoothing kernel
/*
const float kernel[]=float[9] (1,1,1,
								1,1,1,
								1,1,1);
*/
 
//3x3 Gaussian smoothing kernel 
/*
const float kernel[]=float[9] (0,1,0,
								1,5,1,
								0,1,0);
*/ 

 
//Emboss kernels 
/*
const float kernel[]=float[9] (-4,-4, 0,		//emboss in NW direction
								 -4, 12, 0,
								  0, 0, 0);
 
*/
/*
const float kernel[]=float[9] ( 0,-4,-4,		//emboss in NE direction
								  0,12,-4,
								  0, 0, 0);
*/
/*
const float kernel[]=float[9] (0, 0, 0,			//emboss in SE direction
								 0, 12,-4,
								 0,-4,-4);
*/
/*
const float kernel[]=float[9] (  0, 0, 0,		//emboss in SW direction
								  -4, 12, 0,
								  -4,-4, 0);
*/

void main()
{ 
	//determine the inverse of texture size
	vec2 delta = 1.0/textureSize(textureMap,0);
	vec4 color = vec4(0);
	int  index = 8;

	//for convolution we sample in the neighborhood of the current sample point
	//in the given image and accumulated the product of the convolution kernel with the
	//texture sample from the image. At the end, the convolution sum is divided by the
	//total number of pixels in the neighborhood and then added to the current fragment 
	//color. In case of smoothing operation, the obtained colour is directly set as the 
	//filtered colour that is it is not added to the fragment colour.

	//for sharpening and emboss kernels		 
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
	vFragColor =  color + texture(textureMap, vUV); 
	
	 
	/*
	//for smoothing kernels	
	for(int j=-1;j<=1;j++) {
		for(int i=-1;i<=1;i++) {				
			color += kernel[index--]*texture(textureMap, vUV+ (vec2(i,j)*delta));
		}
	}
	color/=9.0;
    vFragColor =  color;
	*/ 
}