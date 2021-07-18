#version 330 core
out vec4 FragColor;

uniform vec3 surfaceColor;
uniform vec3 lightColor;

void main()
{
	FragColor = vec4(surfaceColor * lightColor, 0.0f);
}