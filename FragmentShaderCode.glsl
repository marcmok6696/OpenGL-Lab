#version 430

out vec4 daColor;

in vec3 normalWorld;
in vec3 vertexPositionWorld;
in vec2 UV;

uniform vec4 ambientLight;
uniform vec3 lightPositionWorld1;
uniform vec3 lightPositionWorld2;
uniform vec3 eyePositionWorld;
uniform vec4 lightIntensity;
uniform float n;
uniform sampler2D myTextureSampler;

void main(){
	vec3 lightVectorWorld1 = normalize(lightPositionWorld1 - vertexPositionWorld);
	float brightness1 = dot(lightVectorWorld1, normalize(normalWorld));
	vec4 diffuseLight1 = vec4(brightness1, brightness1, brightness1, 1.0) + lightIntensity;

	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	float brightness2 = dot(lightVectorWorld2, normalize(normalWorld));
	vec4 diffuseLight2 = vec4(brightness2, brightness2, brightness2, 1.0);

	vec3 reflectedLightVectorWorld1 = reflect(-lightVectorWorld1, normalWorld);
	vec3 eyeVectorWorld1 = normalize(eyePositionWorld - vertexPositionWorld);
	float s1 = clamp(dot(reflectedLightVectorWorld1, eyeVectorWorld1), 0, 1);
	s1 = pow(s1, 2);
	vec4 specularLight1 = vec4(s1 * 0.5, s1 * 0.5, s1 * 0.5, 1);

	vec3 reflectedLightVectorWorld2 = reflect(-lightVectorWorld2, normalWorld);
	vec3 eyeVectorWorld2 = normalize(eyePositionWorld - vertexPositionWorld);
	float s2 = clamp(dot(reflectedLightVectorWorld2, eyeVectorWorld2) + n, 0, 1);
	s2 = pow(s2, 10);
	vec4 specularLight2 = vec4(s2 * 0.8 , s2 * 0.8 , 0, 1);

	vec4 MaterialAmbientColor = vec4(texture(myTextureSampler, UV).rgb, 0.5);
	vec4 MaterialDiffuseColor = vec4(texture(myTextureSampler, UV).rgb, 0.5);
	vec4 MaterialSpecularColor = vec4(0.8, 0.8, 0.8, 1.0);

	daColor = MaterialAmbientColor * ambientLight + MaterialDiffuseColor * clamp(diffuseLight1, 0, 1) + MaterialDiffuseColor * clamp(diffuseLight2, 0, 1) + MaterialSpecularColor * specularLight1 + MaterialSpecularColor * specularLight2;
}
