#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output


smooth in vec3 vUV;	//3D texture coordinates form vertex shader 
					//interpolated by rasterizer

//uniforms
uniform sampler3D volume;	//volume dataset
uniform vec4 color;			//constant colour to multiply the 
							//volume density with	

void main()
{
	//Here we sample the volume dataset using the 3D texture coordinates from the vertex shader.
	//Note that since at the time of texture creation, we gave the internal format as GL_RED
	//we can get the sample value from the texture using the red channel. Here, we set all 4
	//components as the sample value in the texture which gives us a shader of grey and multiply
	//the the obtained grey scale value with the given uniform colour to get the final colour.
	vFragColor = texture(volume, vUV).rrrr * color ;	 
}
