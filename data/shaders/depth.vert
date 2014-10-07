#version 420 core

uniform mat4 MVP;

in vec3 a_position;
in int a_normal;

const vec3[6] normals = {
        vec3(0,0,1),
        vec3(0,0,-1),
        vec3(1,0,0),
        vec3(-1,0,0),
        vec3(0,-1,0),
        vec3(0,1,0)
};

void main(void) {
        gl_Position = MVP * vec4(a_position-normals[a_normal]*0.0002f,1.0);
}
