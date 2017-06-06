#version 420

uniform ivec3 transforms[400];
uniform mat4 V;
uniform mat4 VP;

in vec3 a_position;
in int a_normal;
in vec2 a_texCoord;
in float a_light;
in uint draw_index;

out vec2 v_texCoord;
out vec3 v_normal;
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
    v_normal = vec3(V*vec4(normals[a_normal],0.0));
    v_texCoord = a_texCoord/512;
    v_light = a_light;
    gl_Position = VP * vec4(a_position+transforms[draw_index], 1.0);
}
