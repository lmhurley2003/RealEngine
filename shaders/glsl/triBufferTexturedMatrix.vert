#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 fragVertexColor;
layout(location = 1) out vec3 fragNormal;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant, std430) uniform pushConstant {
    mat4 model;
} pc;

void main() {
	gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 1.0);
	fragVertexColor = inColor;
	//TODO if theres no non-uniform scaling, don't need inverse transpose, just model matrix
	fragNormal = (inverse(transpose(pc.model)) * vec4(inNormal, 0.0)).xyz;
}