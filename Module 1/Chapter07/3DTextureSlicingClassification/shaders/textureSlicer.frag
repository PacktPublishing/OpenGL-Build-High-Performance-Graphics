#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

smooth in vec3 vUV;			//3D texture coordinates form vertex shader interpolated by rasterizer

//uniforms
uniform sampler3D volume;	//volume dataset
uniform sampler1D lut;		//transfer function (lookup table) texture

void main()
{
    //Here we sample the volume dataset using the 3D texture coordinates from the vertex shader.
	//Note that since at the time of texture creation, we gave the internal format as GL_RED
	//we can get the sample value from the texture using the red channel. Then, we use the density 
	//value obtained from the volume dataset and lookup the colour from the transfer function texture 
	//by doing a dependent texture lookup.
	vFragColor = texture(lut, texture(volume, vUV).r);
}