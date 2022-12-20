#version 330

layout (location = 0) in vec4 vPosition;
out vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 segColor;
uniform int wired;

void main()
{
	gl_Position = projection * view * model * vPosition;
	color = (wired == 1) ? vec4(0.0, 0.0, 0.0, 0.0) : vec4(segColor, 1.0);
}