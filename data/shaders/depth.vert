#version 420 core

uniform mat4 MVP;

in vec3 a_position;

void main(void) {
        gl_Position = MVP * vec4(a_position,1.0);
}
