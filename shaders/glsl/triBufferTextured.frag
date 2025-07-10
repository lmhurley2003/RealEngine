#version 450

layout(location = 0) in vec3 fragVertexColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 texColor = texture(texSampler, fragTexCoord);
	outColor = vec4(fragVertexColor, 1.0) * texColor;
}