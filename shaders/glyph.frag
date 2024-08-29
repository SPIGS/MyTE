#version 330 core
in vec4 vertexColor;
in vec2 texCoord;

uniform sampler2D uTexture;

void main()
{
	gl_FragColor = texture(uTexture, texCoord) * vertexColor;
}
