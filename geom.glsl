#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 sPos[];
out vec4 gPos[];

void main()
{
	gPos[0] = sPos[0];

//	vec3 coords[3];

//	for (int i = 0; i < 3; i++)
//	{
//		coords[i] = gl_in[i].gl_Position.xyz;
//	}

//	float a = 0.5 * length(cross(coords[0] - coords[1], coords[0] - coords[2]));

//	if (a < 0.001) {

		gl_Position = gl_in[0].gl_Position;
		EmitVertex();

		gl_Position = gl_in[1].gl_Position;
		EmitVertex();

		gl_Position = gl_in[2].gl_Position;
		EmitVertex();

		EndPrimitive();

//	}
}

