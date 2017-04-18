#version 330 core

uniform sampler2DRect depthBlenderTex;	//depth blending output
uniform sampler2DRect frontBlenderTex;	//front blending output
uniform sampler2DRect backBlenderTex;	//back blending output

layout(location = 0) out vec4 vFragColor; //fragment shader output

void main()
{
	//get the front and back blender colors
	vec4 frontColor = texture(frontBlenderTex, gl_FragCoord.xy);
	vec3 backColor = texture(backBlenderTex, gl_FragCoord.xy).rgb; 

	// front + back
	//composite the front and back blending results
	vFragColor.rgb = frontColor.rgb + backColor * frontColor.a;
	
	// front blender
	//vFragColor.rgb = frontColor + vec3(alphaMultiplier);
	
	// back blender
	//vFragColor.rgb = backColor;
}