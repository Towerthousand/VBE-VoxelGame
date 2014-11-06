#version 420

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

in vec3 a_position;
in int a_normal;
in vec2 a_texCoord;
in vec4 a_color;
in float a_light;

out vec3 v_normal;
out vec2 v_texCoord;
out vec4 v_color;
out float v_light;

const vec3[6] normals = {
	vec3(0,0,1),
	vec3(0,0,-1),
	vec3(1,0,0),
	vec3(-1,0,0),
	vec3(0,-1,0),
	vec3(0,1,0)
};

void main(void) {
	v_normal = normalize(vec4(V*M*vec4(normals[a_normal], 0.0)).xyz);
	v_texCoord = a_texCoord/512;
        v_light = a_light;
        gl_Position = MVP * vec4(a_position, 1.0);
}
