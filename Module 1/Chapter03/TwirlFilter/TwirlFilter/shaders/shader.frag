#version 330 core
 
layout(location=0) out vec4 vFragColor;	//fragment shader output

//input from the vertex shader
smooth in vec2 vUV;						//2D texture coordinates

//shader uniforms
uniform sampler2D textureMap;			//the image to twirl
uniform float twirl_amount;				//teh amount of twirl

void main()
{
	//get the shifted UV coordinates so that the origin of twirl is at the center of image
	vec2 uv = vUV-0.5;
	
	//get the angle from the shifter UV coordinates
   float angle = atan(uv.y, uv.x);

   //get the radius using the Euclidean distance of the shifted texture coordinate
   float radius = length(uv);

   //increment angle by product of twirl amount and radius
   angle+= radius*twirl_amount; 

   //convert to Cartesian coordinates
   vec2 shifted = radius* vec2(cos(angle), sin(angle));

   //shift by 0.5 to bring it back to original unshifted position
   vFragColor = texture(textureMap, (shifted+0.5)); 
}