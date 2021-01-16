#version 330 core
out vec4 FragColor;

uniform vec3 surfaceColor;

void main()
{
	FragColor = vec4(surfaceColor, 1.0f);
}