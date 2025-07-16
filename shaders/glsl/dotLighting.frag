#version 450

layout(location = 0) in vec4 fragVertexColor;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 light = mix(vec3(0,0,0), vec3(1,1,1),
				dot(fragNormal, vec3(0,0,1)) * 0.5 + 0.5);
	outColor = vec4(light, 1.0f) * fragVertexColor;
}