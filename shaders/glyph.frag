#version 330 core
in vec4  v_color;
in vec2  v_uv;
in float v_texindex;

layout(location = 0) out vec4 f_color;
uniform sampler2D u_tex[8];

void main() {
	vec4 sampled = texture(u_tex[int(v_texindex)], v_uv);
	f_color = vec4(v_color.rgb, v_color.a * sampled.r);
}