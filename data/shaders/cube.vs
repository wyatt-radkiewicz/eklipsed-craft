#version 330 core
in vec3 apos;
in vec3 anorm;
in vec2 auv;
in int auv_idx;
in mat4 iworld;

out vec2 stexcoords;
out vec3 snormal;

uniform mat4 uproj;
uniform mat4 uview;
uniform sampler2D utexuvs;
uniform int utexw;
uniform float utex_xstep;
uniform float utex_ystep;

void main() {
	gl_Position = uproj * uview * iworld * vec4(apos.xyz, 1.0);
	snormal = (iworld * vec4(anorm, 1.0)).xyz;
	vec4 packed = texture(utexuvs, vec2(
		(float(id % utexw) + 0.5) * utex_xstep,
		(float(id / utexh) + 0.5) * utex_ystep,
	));
	stexcoords = (auv * packed.zw) + packed.xy;
}

