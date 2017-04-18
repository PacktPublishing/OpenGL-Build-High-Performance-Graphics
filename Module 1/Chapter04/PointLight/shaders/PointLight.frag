#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output
 
//uniforms
uniform mat4 MV;				//modelview matrix
uniform vec3 light_position;	//light position in object space
uniform vec3 diffuse_color;		//diffuse colour of surface

//inputs from the vertex shader
smooth in vec3 vEyeSpaceNormal;		//interpolated eye space normal
smooth in vec3 vEyeSpacePosition;	//interpolated eye space position

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main() { 
	//multiply the object space light position with the modelview matrix 
	//to get the eye space light position
	vec3 vEyeSpaceLightPosition = (MV*vec4(light_position,1)).xyz;
	//get the light vector
	vec3 L = (vEyeSpaceLightPosition-vEyeSpacePosition);
	//get the distance of light source
	float d = length(L);
	//normalize the light vector
	L = normalize(L);
	//calcualte the diffuse component
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	
	//apply attenuation
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
	//return the product of the diffuse component with the diffuse color
	//as the final output fragment colour
	vFragColor = diffuse*vec4(diffuse_color,1);	 
}