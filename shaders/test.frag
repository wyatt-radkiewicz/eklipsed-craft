#version 450

layout(location = 0) in vec3 shdr_frag_color;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(shdr_frag_color, 1.0);
}
