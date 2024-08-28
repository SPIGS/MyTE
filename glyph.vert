#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTextCoord;
layout (location = 3) in float aTextIndex;

out vec4 vertexColor;
out vec2 texCoord;

void main()
{
	vertexColor = aColor;
	texCoord = aTextCoord;
	gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
 }
