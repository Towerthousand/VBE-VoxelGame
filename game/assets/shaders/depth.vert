#version 420 core

in vec3 a_position;
in int a_normal;
in uint draw_index;

out uint g_draw_index;

const vec3[6] normals = {
    vec3(0,0,1),
    vec3(0,0,-1),
    vec3(1,0,0),
    vec3(-1,0,0),
    vec3(0,-1,0),
    vec3(0,1,0)
};

void main(void) {
    g_draw_index = draw_index;
    gl_Position = vec4(a_position-normals[a_normal]*0.0002f,1.0);
}
