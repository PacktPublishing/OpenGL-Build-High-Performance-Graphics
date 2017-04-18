#version 330 core
  
uniform sampler2D normalTex;	//normal texture with normals in eye space
uniform sampler2D depthTex;		//depth texture from the FBO depth attachment
uniform sampler2D noiseTex;		//noise texture

//uniforms
uniform vec2 viewportSize;		//viewport size
uniform vec2 invViewportSize;	//inverse of viewport size
uniform vec2 samples[16];		//a set of 16 sample locations 
uniform mat4 invP;				//inverse of projection matrix
uniform float radius;			//occlusion radius for sampling of depth
  
smooth in vec2 vUV;	//interpolated texture coordinates

layout(location=0) out vec4 vFragColor;	//fragment shader output
 
//shader constants
const float g_scale = 1;			//controls the radius of occlusion
const float g_bias =  0;			//>0 lighter <0 darker
const float g_intensity = 1.5;		//>1 makes the shadows darker
const int   NUM_SAMPLES = 6;		//number of samples
const float INV_NUM_SAMPLES = 1.0/NUM_SAMPLES;	//inverse number of samples
const float random_size = 64;					//random texture size
const float DEPTH_TOLERANCE = 0.00001;			//depth bias

//returns pseudo random number
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
  
//returns the amount of ambient occlusion between point (p) and (p1) in the 
//given direction (cnorm)
float calcAO(in vec4 p, in vec4 p1, in vec3 cnorm)
{
	vec3 diff = p1.xyz - p.xyz - DEPTH_TOLERANCE; 
	vec3 v = normalize(diff);
	float d = length(diff)*g_scale; 
	return max(0.1,dot(cnorm,v)-g_bias)*(1.0/(1.0+d))*g_intensity;
}


void main()
{  
	//get the current depth
	float depth = texture(depthTex, vUV).r; 

	//if depth is within range
	if(depth<1.0)
	{ 
		//get the normal (when we stoed the normal in first step shader, we change
		//the normal range to 0 to 1. Now we change it back to -1 to 1 range
		vec3 n = normalize(texture(normalTex, vUV).xyz*2.0 - 1.0);

		//get the position from the texture cooridnates and depth
		//and do a homogeneous division
		vec4 p = invP*vec4(vUV,depth,1);
		p.xyz /= p.w;
		 
		
		//if we want to calculate noise pseudorandom value in shader
		vec2 random = normalize(vec2(rand(vUV.xy), rand(vUV.yx))*2.0 -1.0);
		
		//if we want to use the noise texture to get a pseudorandom value
		//vec2 random = normalize(texture(noiseTex, viewportSize/random_size * vUV).rg * 2.0 - 1.0); 	 

		//loop through all samples and estimate the ambient occlusion amount
		float ao = 0.0;		  
		for(int i = 0; i < NUM_SAMPLES; i++)
		{
			//randomly offset the sample point
			float npw = ( radius * samples[i].x * random.x);
			float nph = ( radius * samples[i].y * random.y);
			
			//get the new texture coordinate and read the depth value 
			//at the new sample point. From this new depth, get a new
			//position and pass it to the calcAO function to get the 
			//occlusion amount
			vec2 uv = vUV + vec2(npw, nph); 
			vec4 p0 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p0.xyz /= p0.w;

			uv = vUV + vec2(npw, -nph); 
			vec4 p1 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p1.xyz /= p1.w;
			 
			uv = vUV + vec2(-npw, nph); 
			vec4 p2 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p2.xyz /= p2.w;

			uv = vUV + vec2(-npw, -nph); 
			vec4 p3 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p3.xyz /= p3.w;

		 	uv = vUV + vec2(0, nph); 
			vec4 p4 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p4.xyz /= p4.w;

			uv = vUV + vec2(0, -nph); 
			vec4 p5 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p5.xyz /= p5.w;

			uv = vUV + vec2(npw, 0); 
			vec4 p6 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p6.xyz /= p6.w;

			uv = vUV + vec2(-npw, 0); 
			vec4 p7 = invP * vec4(vUV,  texture(depthTex, uv ).r, 1.0);			p7.xyz /= p7.w;
			 
			//get the ambient occlusion amount 
			ao += calcAO(p0, p, n);
			ao += calcAO(p1, p, n);
			ao += calcAO(p2, p, n);
			ao += calcAO(p3, p, n);
			ao += calcAO(p4, p, n);
			ao += calcAO(p5, p, n);
			ao += calcAO(p6, p, n);
			ao += calcAO(p7, p, n);  
			 
		}

		//normalize the ambient occlusion amount
		ao *= INV_NUM_SAMPLES /8.0  ;
		
		//set the ambient occlusion value in the alpha channel as we will blend
		//this result over the output 
		vFragColor = vec4(vec3(0), ao); 
	} 
}
	