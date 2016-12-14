#version 150

in vec3 position;
in vec2 texcoord;
in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 sPos;

void main()
{
    gl_Position = proj * view * model * vec4(position, 1.0);
	sPos = 2.0 * vec4(position, 1.0);
}


