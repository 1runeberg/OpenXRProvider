#version 330 core
out vec4 FragColor;

in vec3 VertColor;
uniform vec3 surfaceColor;

void main()
{
	FragColor = vec4(VertColor * surfaceColor, 1.0f);
}