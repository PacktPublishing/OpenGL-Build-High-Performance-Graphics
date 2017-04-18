#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//uniforms
uniform mat4 MV;					//modelview matrix
uniform sampler2DShadow shadowMap;	//shadowmap texture
uniform vec3 light_position;		//light position in object space
uniform vec3 diffuse_color;			//surface's diffuse colour
uniform bool bIsLightPass;			//flag to indicate the light pass
									//we donot cast shadows in light pass

//inputs from the vertex shader
smooth in vec3 vEyeSpaceNormal;		//interpolated eye space normal
smooth in vec3 vEyeSpacePosition;	//interpolated eye space position
smooth in vec4 vShadowCoords;		//interpolated shadow coordinates

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main() { 
	//if this is the light pass, we donot cast shadows and simply return
	//since we only require depth which is stored in the depth attachment
	//of FBO
	if(bIsLightPass)
		return;
		 
	//get light position in eye space
	vec4 vEyeSpaceLightPosition = MV*vec4(light_position,1);
	
	//get the light vector
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition);

	//get the distance of the light source
	float d = length(L);

	//normalize the light vector
	L = normalize(L);

	//calculate the diffuse component and apply light attenuation 
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	float diffuse = max(0, dot(vEyeSpaceNormal, L)) * attenuationAmount;	
	 
	//if the homogeneous coordinate is > 1, we are in the forward half
	//so we should cast shadows. If this check is removed, you will see 
	//shadows on both sides of the light when the light is very close to 
	//the plane. Try removing this to see what I mean.
	if(vShadowCoords.w>1) 
	{
		//check the shadow map texture to see if the fragment is in shadow
		float shadow = textureProj(shadowMap, vShadowCoords);
		//darken the diffuse component apprpriately
		diffuse = mix(diffuse, diffuse*shadow, 0.5); 
	}

	//return the final colour by multiplying the diffuse colour with the diffuse component
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}