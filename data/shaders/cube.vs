#version 330 core
in vec3 apos;
in vec3 anorm;
in vec2 auv;
in int afaceid;
in mat4 iworld;
in vec4 ipz;
in vec4 inz;
in vec4 ipy;
in vec4 iny;
in vec4 ipx;
in vec4 inx;

out vec2 stexcoords;
out vec3 snormal;

uniform mat4 uproj;
uniform mat4 uview;

void main() {
	vec4 uvt;
	int id = gl_InstanceID * 6 + afaceid;

	gl_Position = uproj * uview * iworld * vec4(apos.xyz, 1.0);
	snormal = (iworld * vec4(anorm, 1.0)).xyz;

	switch (afaceid) {
	case 0: uvt = ipz; break;
	case 1: uvt = inz; break;
	case 2: uvt = ipy; break;
	case 3: uvt = iny; break;
	case 4: uvt = ipx; break;
	case 5: uvt = inx; break;
	}

	stexcoords = (auv * uvt.zw) + uvt.xy;
}

