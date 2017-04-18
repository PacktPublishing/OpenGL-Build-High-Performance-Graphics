#version 150

in vec3 position;
in vec3 color_in;
out vec3 color;

void main() {
   color = color_in;
   gl_Position = vec4(position, 1.0);
}
