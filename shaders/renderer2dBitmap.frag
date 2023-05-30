#version 460 core

in vec3 FragPos;
in vec2 texCoord;

uniform vec4 color;
uniform sampler2D tex;

//out vec4 FragColor;
//
//void main() {
//	FragColor = color * vec4(1.0, 1.0, 1.0, texture(tex, texCoord).b);
//	//float w = 0.025;
//	//float distance = texture(tex, texCoord).r;
//	//float alpha = smoothstep(0.5 - w, 0.5 + w, distance);
//	//FragColor = vec4(color.rgb, alpha * color.a);
//}

layout(location = 0, index = 0) out vec4 fragcolor;
layout(location = 0, index = 1) out vec4 colorMask;

void main() {
	fragcolor = color;
	colorMask = vec4(texture(tex, texCoord).rgb, 1.0);
}