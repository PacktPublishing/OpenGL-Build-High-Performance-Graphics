#version 330 core
 
layout(location = 0) in vec3 vVertex;	 //vertex position
layout(location = 1) in vec3 vNormal;	 //vertex normal
layout(location = 2) in vec2 vUV;		 //vertex uv coordinates
 
//uniforms for projection, modelview and normal matrices
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;
uniform vec3 light_position; // light position in object space

//shader outputs to the fragment shader
smooth out vec2 vUVout;			//texture coordinates
smooth out vec4 diffuse;		//diffuse colour

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main()
{
	//output the texture coordinates
	vUVout=vUV; 

	//multiply the object space normal with the normal matrix to get 
	//the eye space normal
	vec3 tmpN = normalize(N*vNormal);  

	//multiply the object space light position with the modelview matrix 
	//to get the eye space light position  
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));

	//multiply the object space vertex position with the modelview matrix 
	//to get the eye space position  
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1);

	//get the light vector
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);

	//get distance of light
	float d = length(L);

	//normalize the light vector
	L = normalize(L);

	//calcualte the attenuation amount and apply attenuation
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	float nDotL = max(0, dot(L,tmpN)) * attenuationAmount; 
	diffuse = vec4(nDotL);
    
	//multiply the projection matrix with the eye space position to get
	//the clipspace postion
	gl_Position = P*vEyeSpacePosition;
}