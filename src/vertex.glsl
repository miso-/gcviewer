#version 130

uniform mat4 proj_view_matrix;
uniform mat3 normal_matrix;
uniform vec4 global_color;

in vec3 position;		// gl_Vertex
in vec3 normal;			// gl_Normal

out vec4 color;

void main()
{
	vec3 N = normalize(normal_matrix * normal);
	gl_Position = proj_view_matrix * vec4(position, 1.0);

	vec3 L = vec3(0.0, 0.0, 1.0);

	vec4 ambient = global_color * 0.7;
	vec4 diffuse = global_color * max(dot(N,L), 0.0);

	color = ambient + diffuse;
}
