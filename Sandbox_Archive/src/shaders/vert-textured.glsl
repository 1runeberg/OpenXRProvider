#version 330 core
layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in mat4 mvp;

out vec2 TexCoord;

void main()
{
	gl_Position = mvp* vec4(vertPosition, 1.0);
	TexCoord = vec2(texCoord.x, texCoord.y);
}
