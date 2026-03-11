#version 330 core

out vec4 frag_col;

in vec4 o_col;

void main()
{
	frag_col = o_col;
}
