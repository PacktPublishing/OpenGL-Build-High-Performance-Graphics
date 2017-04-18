#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output
 
//uniforms
uniform mat4 MV;				//modelview matrix
uniform vec3 light_position;	//light position in object space
uniform vec3 diffuse_color;		//diffuse colour of surface
uniform vec3 specular_color;	//specular colour of surface
uniform float shininess;		//specular shininess 

//inputs from the vertex shader
smooth in vec3 vEyeSpaceNormal;		//interpolated eye space normal
smooth in vec3 vEyeSpacePosition;	//interpolated eye space position

//shader constant
const vec3 vEyeSpaceCameraPosition = vec3(0,0,0);

void main() { 
	//multiply the object space light position with the modelview matrix 
	//to get the eye space light position
	vec3 vEyeSpaceLightPosition = (MV * vec4(light_position,1)).xyz;

	//normalize the eye space normal
	vec3 N = normalize(vEyeSpaceNormal);	
	//get the light vector and normalize it
	vec3 L = normalize(vEyeSpaceLightPosition-vEyeSpacePosition);
	//get the view vector and normalize it
	vec3 V = normalize(vEyeSpaceCameraPosition.xyz-vEyeSpacePosition.xyz);
	//get the half vector between light and view vector and normalize it
	vec3 H = normalize(L+V);
	//calculate the diffuse component
	float diffuse = max(0, dot(N, L));
	//calculat the specular component
	float specular = max(0, pow(dot(N, H), shininess));

	//output the sum of diffuse and specular colours with their respective 
	//component as the final fragment colour
	vFragColor = diffuse*vec4(diffuse_color,1) + specular*vec4(specular_color, 1);
}