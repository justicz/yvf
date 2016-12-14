#version 150

in vec3 position;

in vec4 gPos[];

void main()
{
    gl_FragColor = gPos[0];
}

