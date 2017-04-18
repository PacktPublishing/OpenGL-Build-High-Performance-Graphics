#version 330 core

layout (location=0) out vec4 vFragColor;	//fragment shader output

//uniforms
uniform vec3 light_position;	//light position in eye space
uniform vec3 spot_direction;	//light spot position in eye space
uniform float spot_cutoff;		//spot light cutoff
uniform float spot_exponent;	//spot light spot exponent
uniform vec3 diffuse_color;		//surface diffuse colour

//inputs varyings from the vertex shader
smooth in vec3 vEyeSpaceNormal;    //interpolated eye space normal
smooth in vec3 vEyeSpacePosition;  //interpolated eye space position

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation
 
void main() { 
	//get the light vector
	vec3 L = (light_position.xyz-vEyeSpacePosition);
	//get the light distance
	float d = length(L);
	//normalize the light vector
	L = normalize(L);
	//normalize the spot direction
	vec3 D = normalize(spot_direction);

	//calculate the overlap between the spot and the light direction
	vec3 V = -L;
	float diffuse = 1;	  
	float spotEffect = dot(V,D);

	//if the spot effect is > cutoff we shade the surface
	if(spotEffect > spot_cutoff) {
		//get the diffuse component
		diffuse = max(0, dot(vEyeSpaceNormal, L));	
		//calculate the spot effect
		spotEffect = pow(spotEffect, spot_exponent);
		//apply attenuation
		float attenuationAmount = spotEffect/(k0 + (k1*d) + (k2*d*d));
		diffuse *=  attenuationAmount;
		//return the diffuse colour multiplied by the diffuse component of the surface
		vFragColor = diffuse*vec4(diffuse_color,1);	 
	}  
}