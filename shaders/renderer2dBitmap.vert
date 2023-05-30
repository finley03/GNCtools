#version 460 core

// inputs of vertex buffer object
// same layout as defined in object mesh
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

// uniform inputs
uniform vec2 offset;
uniform float zIndex;

// matrix inputs
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;

// main function
void main() {
	// calculate vertex position on screen
	gl_Position = projection * view * vec4(aPosition + offset, zIndex, 1.0);
	texCoord = aTexCoord;
}
