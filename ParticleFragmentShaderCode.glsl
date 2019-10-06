#version 430

out vec4 color;

in vec4 particleColor;

void main(){
	color = particleColor;
}