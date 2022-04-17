#version 330

in float age;
uniform sampler2D texture0;
out vec4 outColor;

void main()
{
	outColor = vec4(0.5, 0.5, 0.5, 1 - age);
}
