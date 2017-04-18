#version 150 core

in vec2 UV;
out vec4 color;
uniform sampler2D textureSampler;
in vec4 color_based_on_position;

void main(){
	color = 0.5f*texture(textureSampler, UV).rgba+0.5f*color_based_on_position;
}