#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nrm;
layout(location = 2) in vec4 a_col;

out vec4 o_col;

void main()
{
	gl_Position = vec4(a_pos, 1.0);
	o_col	    = a_col;
}
