#version 330 core

layout(location = 0) out vec4 vFragColor0;	//output to target 0
layout(location = 1) out vec4 vFragColor1;	//output to target 1
layout(location = 2) out vec4 vFragColor2;	//output to target 2	
 
//uniforms
uniform vec4 vColor;		//solid colour of the cube
uniform sampler2DRect  depthBlenderTex;	//depth blending output
uniform sampler2DRect  frontBlenderTex;	//front blending output
uniform float alpha;	//fragment alpha

#define MAX_DEPTH 1.0	//max depth value to clear the depth with
 

void main()
{
	//get the current fragment depth
	float fragDepth = gl_FragCoord.z;
	//get the depth value from the depth blending output
	vec2 depthBlender = texture(depthBlenderTex, gl_FragCoord.xy).xy;
	//get the front blending output
	vec4 forwardTemp = texture(frontBlenderTex, gl_FragCoord.xy);

	// Depths and 1.0-alphaMult always increase
	// so we can use pass-through by default with MAX blending
	vFragColor0.xy = depthBlender;
	
	// Front colors always increase (DST += SRC*ALPHA_MULT)
	// so we can use pass-through by default with MAX blending
	vFragColor1 = forwardTemp;
	
	// Because over blending makes color increase or decrease,
	// we cannot pass-through by default.
	// Each pass, only one fragment can a color greater than 0
	vFragColor2 = vec4(0.0);

	float nearestDepth = -depthBlender.x;
	float farthestDepth = depthBlender.y;
	float alphaMultiplier = 1.0 - forwardTemp.w;

	if (fragDepth < nearestDepth || fragDepth > farthestDepth) {
		// Skip this depth in the peeling algorithm
		vFragColor0.xy = vec2(-MAX_DEPTH);
		return;
	}
	
	if (fragDepth > nearestDepth && fragDepth < farthestDepth) {
		// This fragment needs to be peeled again
		vFragColor0.xy = vec2(-fragDepth, fragDepth);
		return;
	}	 
	
	// If we made it here, this fragment is on the peeled layer from last pass
	// therefore, we need to shade it, and make sure it is not peeled any farther
	vFragColor0.xy = vec2(-MAX_DEPTH);
	
	//if the fragment depth is the nearest depth, we blend the colour 
	//to the second attachment
	if (fragDepth == nearestDepth) {
		vFragColor1.xyz += vColor.rgb * alpha * alphaMultiplier;
		vFragColor1.w = 1.0 - alphaMultiplier * (1.0 - alpha);
	} else {
		//otherwise we write to the thrid attachment
		vFragColor2 += vec4(vColor.rgb, alpha);
	}
}