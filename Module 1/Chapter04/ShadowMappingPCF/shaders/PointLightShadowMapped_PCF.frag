#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//uniforms
uniform mat4 MV;						//modelview matrix
uniform sampler2DShadow shadowMap;		//shadowmap texture
uniform vec3 light_position;			//light position in object space
uniform vec3 diffuse_color;				//surface's diffuse colour
uniform bool bIsLightPass;				//flag to indicate the light pass
										//we donot cast shadows in light pass

//inputs from the vertex shader
smooth in vec3 vEyeSpaceNormal;		//interpolated eye space normal
smooth in vec3 vEyeSpacePosition;	//interpolated eye space position
smooth in vec4 vShadowCoords;		//interpolated shadow coordinates

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation
 
//uncomment one of these to enable the corresponding PCF mode
//#define STRATIFIED_3x3
//#define STRATIFIED_4x4 
#define RANDOM_SAMPLING

#ifdef RANDOM_SAMPLING
//pseudorandom number generator
float random(vec4 seed) {
	float dot_product = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}
#endif

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
	if(vShadowCoords.w>1) {

		//In case of PCF, we take a number of shadow map samples and then
		//average their contributions. The average value is then used to 
		//darken/lighten the shading. For this case, we have a separate
		//function textureProjOffset that accepts an offset from the given
		//shadow coordinate and returns the shadow comparison result.

		float sum = 0;
		float shadow = 1;

		//using 3x3 neighborhood
		#ifdef STRATIFIED_3x3
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 0, 2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 0));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 2));
		shadow = sum/9.0;
		#endif
		
		//using 4x4 neighborhood
		#ifdef STRATIFIED_4x4
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1,-2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-2));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1,-1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2,-1));

		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1, 1));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 1));
 
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-2, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2(-1, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 1, 2));
		sum += textureProjOffset(shadowMap, vShadowCoords, ivec2( 2, 2));

		shadow = sum/16.0;
		#endif

		#ifdef RANDOM_SAMPLING	 
		for(int i=0;i<16;i++) {
			float indexA = (random(vec4(gl_FragCoord.xyx, i))*0.25);
			float indexB = (random(vec4(gl_FragCoord.yxy, i))*0.25); 
			sum += textureProj(shadowMap, vShadowCoords+vec4(indexA, indexB, 0, 0));
		}
		shadow = sum/16.0;
		#endif

		//based on the value of shadow, the fragment is lightened/darkened
		diffuse = mix(diffuse, diffuse*shadow, 0.5); 
	}
	//return the final colour by multiplying the diffuse colour with the diffuse component
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}