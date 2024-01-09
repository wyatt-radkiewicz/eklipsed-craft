#version 330 core
in vec3 apos;
in vec3 anorm;
in vec2 auv;
in int afaceid;
in mat4 iworld;

out vec2 stexcoords;
out vec3 snormal;

uniform mat4 uproj;
uniform mat4 uview;
uniform samplerBuffer uuvs;

void main() {
	vec4 uvt;
	int id = gl_InstanceID * 6 + afaceid;

	gl_Position = uproj * uview * iworld * vec4(apos.xyz, 1.0);
	snormal = (iworld * vec4(anorm, 1.0)).xyz;

	uvt = vec4(0.0, 0.0, 1.0, 1.0);
	uvt = texelFetch(uuvs, id);
	stexcoords = (auv * uvt.zw) + uvt.xy;
}

