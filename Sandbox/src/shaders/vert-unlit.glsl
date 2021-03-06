#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in mat4 mvp;

out vec3 VertColor;

void main()
{
	gl_Position = mvp * vec4(position, 1.0);
	VertColor = color;
}
