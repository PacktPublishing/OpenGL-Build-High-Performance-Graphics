#version 150 core

out vec4 color;
in vec4 color_based_on_position;

void main(){
	//pipe the output directly
	color = color_based_on_position;
}
