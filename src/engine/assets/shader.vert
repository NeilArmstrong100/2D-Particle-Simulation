#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float radius;

out vec3 color;

uniform vec2 cameraPos;
uniform float zoom;
uniform float aspectRatio;

void main()
{
	gl_Position = vec4(((aPos.x + cameraPos.x) * zoom) / aspectRatio, (aPos.y + cameraPos.y) * zoom, 0.0, 1.0);
	gl_PointSize = radius * zoom;
	color = aColor;
}