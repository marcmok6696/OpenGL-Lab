#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

uniform mat4 modelTransformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main(){
	vec4 newPosition = modelTransformMatrix * vec4(position, 1.0);
	gl_Position = projectionMatrix * viewMatrix * newPosition;

	vec4 normal_temp = modelTransformMatrix * vec4(normal, 0);
	normalWorld = normal_temp.xyz;
	vertexPositionWorld = newPosition.xyz;

	UV = vertexUV;
}