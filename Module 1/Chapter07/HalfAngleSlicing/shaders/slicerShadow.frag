#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

smooth in vec3 vUV;			//interpolated texture coordinates
smooth in vec4 vLightUVW;	//interpolated shadow texture sampling coordinates

//uniforms
uniform sampler3D volume;		//the volume dataset
uniform sampler2D shadowTex;	//the shadow texture
uniform vec4 color;				//the colour of light	

void main()
{  
	//get the light intensity from the shadow texture
    vec3 lightIntensity =  textureProj(shadowTex, vLightUVW.xyw).xyz;
	
	//get the volume density from the volume dataset
	float density = texture(volume, vUV).r;

	//remove low density values to remove artefacts
	if(density > 0.1) {
		
		//get alpha from the volume density
		float alpha = clamp(density, 0.0, 1.0);
		
		//multiply the alpha with the alpha of the colour
		alpha *= color.a;

		//return the final colour by multiplying the colour with 
		//light intensity and alpha
		vFragColor = vec4(color.xyz*lightIntensity*alpha, alpha);
	} 
}