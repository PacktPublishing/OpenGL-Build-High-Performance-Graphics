#version 330 core
  
uniform sampler2D textureMap;		//the texture on the submesh
uniform float useDefault;			//0-> submesh has texture, 1-> submesh has no texture
uniform vec3 light_position;		//light position in object space
uniform mat4 MV;					//modelview matrix (used here to get the eye space light position

smooth in vec3 vEyeSpaceNormal;		//eye space normal from the vertex shader interpolated by rasterizer
smooth in vec3 vEyeSpacePosition;	//eye space position from the vertex shader interpolated by rasterizer
smooth in vec2 vUVout;				//texture coordinates interpolated by rasterizer

layout(location=0) out vec4 vFragColor;	//fragment shader output

//shader constants for attenuation
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main()
{ 
	//get the eye space light position
	vec4 vEyeSpaceLightPos = MV * vec4(light_position,1);

	//get the light vector
	vec3 L = (vEyeSpaceLightPos.xyz-vEyeSpacePosition);

	//get the distance of the light source
	float d = length(L);

	//normalize the light vector 
	L = normalize(L);

	//get the diffuse component
	float diffuse = max(0, dot(normalize(vEyeSpaceNormal), L));	

	//apply attenuation
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;

	//output the combined colour
	//we use mix here to combine the diffuse+texture colour with a default colour (white)
	//whe usedefault is 1, the mix function will return white colour otherwise, it will return 
	//diffuse+texture colour
	vFragColor = diffuse*mix(texture(textureMap, vUVout), vec4(1), useDefault);
}