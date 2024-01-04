#version 330 core
out vec4 FragColor;

in vec2 stexcoords;
in vec3 snormal;

uniform sampler2D utexture;

void main() {
    FragColor = texture(utexture, stexcoords);
} 
