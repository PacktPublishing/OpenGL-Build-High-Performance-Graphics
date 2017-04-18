#version 330 core

layout(location=0) out vec4 vFragColor;	//fragment shader output

//shader uniforms
uniform mat4 MV;					//modelview matrix
uniform sampler2D  shadowMap;		//shadowmap texture
uniform vec3 light_position;		//light position in object space
uniform vec3 diffuse_color;			//surface's diffuse colour
									 
//inputs from the vertex shader		 
smooth in vec3 vEyeSpaceNormal;		//interpolated eye space normal
smooth in vec3 vEyeSpacePosition;	//interpolated eye space position
smooth in vec4 vShadowCoords;		//interpolated shadow coordinates

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main() {   

	//get light position in eye space
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));
	
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
		
		//divide the shadow coordinate by homogeneous coordinate
		vec3 uv = vShadowCoords.xyz/vShadowCoords.w;
		
		//get the depth value
		float depth = uv.z;
		
		//read the moments from the shadow map texture
		vec4 moments = texture(shadowMap, uv.xy); 

		//calculate variance from the moments
		float E_x2 = moments.y;
		float Ex_2 = moments.x*moments.x;
		float var = E_x2-Ex_2;

		//bias the variance
		var = max(var, 0.00002);

		//subtract the fragment depth from the  first moment
		//divide variance by the squared difference value
		//to get the maximum probability of fragment to be in shadow
		float mD = depth-moments.x;
		float mD_2 = mD*mD; 
		float p_max = var/(var+ mD_2); 

		//darken the diffuse component if the current depth is less than or equal
		//to the first moment and the retured value is less than the calculated
		//maximum probability
		diffuse *= max(p_max, (depth<=moments.x)?1.0:0.2); 
	}
	//return the final colour by multiplying the diffuse colour with the diffuse component
	vFragColor = diffuse*vec4(diffuse_color, 1);	 
}