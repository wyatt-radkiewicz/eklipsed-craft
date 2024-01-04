#version 330 core
in vec3 apos;
in vec3 anorm;
in vec2 auv;
in mat4 iworld;
in vec4 iuv;

out vec2 stexcoords;
out vec3 snormal;

uniform mat4 uproj;
uniform mat4 uview;

void main() {
	gl_Position = uproj * uview * iworld * vec4(apos.xyz, 1.0);
	snormal = (iworld * vec4(anorm, 1.0)).xyz;
	stexcoords = (auv * iuv.zw) + iuv.xy;
}

