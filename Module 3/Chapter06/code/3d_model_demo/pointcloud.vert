#version 150 core

// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_modelspace;

// Output data ; will be interpolated for each fragment.
out vec4 color_based_on_position;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

//heat map generator
vec4 heatMap(float v, float vmin, float vmax){
    float dv;
    float r=1.0f, g=1.0f, b=1.0f;
	if (v < vmin)
		v = vmin;
	if (v > vmax)
		v = vmax;
	dv = vmax - vmin;
	if (v < (vmin + 0.25f * dv)) {
		r = 0.0f;
		g = 4.0f * (v - vmin) / dv;
	} else if (v < (vmin + 0.5f * dv)) {
		r = 0.0f;
		b = 1.0f + 4.0f * (vmin + 0.25f * dv - v) / dv;
	} else if (v < (vmin + 0.75f * dv)) {
		r = 4.0f * (v - vmin - 0.5f * dv) / dv;
		b = 0.0f;
	} else {
		g = 1.0f + 4.0f * (vmin + 0.75f * dv - v) / dv;
		b = 0.0f;
	}
    //with 0.2 transparency - can be dynamic if we pass in variables
    return vec4(r, g, b, 0.2f);
}

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace, 1.0f);
	//change the -1.0f and 1.0f to the max and min range of Z
	color_based_on_position = heatMap(vertexPosition_modelspace.z, -1.0f, 1.0f);
}

