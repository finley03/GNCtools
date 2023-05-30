#version 460 core

in vec3 FragPos;

uniform vec4 color;

out vec4 FragColor;

void main() {
	FragColor = color;
}
