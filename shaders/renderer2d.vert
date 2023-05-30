#version 460 core

// inputs of vertex buffer object
// same layout as defined in object mesh
layout (location = 0) in vec2 aPosition;

// uniform inputs
uniform vec2 offset;
uniform float zIndex;

// matrix inputs
uniform mat4 view;
uniform mat4 projection;

// main function
void main() {
	// calculate vertex position on screen
	gl_Position = projection * view * vec4(aPosition + offset, zIndex, 1.0);
}
