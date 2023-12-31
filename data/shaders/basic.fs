#version 330 core
out vec4 FragColor;

in vec2 s_texcoords;

uniform sampler2D u_texture;

void main()
{
    FragColor = texture(u_texture, s_texcoords);
} 
