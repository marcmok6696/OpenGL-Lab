#version 430

layout(location = 0) in vec3 vertices;
layout(location = 1) in vec4 info;
layout(location = 2) in vec4 color;

uniform mat4 modelTransformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec4 particleColor;

void main(){
	vec3 eyeRight = vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	vec3 eyeUp = vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

    vec3 position = info.xyz + eyeRight * vertices.x * info.w + eyeUp * vertices.y * info.w;
	gl_Position = projectionMatrix * viewMatrix * modelTransformMatrix * vec4(position, 1.0);

	particleColor = color;
}