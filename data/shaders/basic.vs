#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 i_mat;
layout (location = 7) in vec2 i_uvpos;
layout (location = 8) in vec2 i_uvsize;

out vec2 s_texcoords;
out vec3 s_normal;

uniform mat4 u_proj;
uniform mat4 u_view;

void main()
{
    gl_Position = u_proj * u_view * i_mat * vec4(a_pos, 1.0); 
    s_texcoords = vec2(a_uv.x * i_uvsize.x, a_uv.y * i_uvsize.y) + i_uvpos;
	s_normal = vec4(i_mat * vec4(a_normal, 1.0)).xyz;
}
