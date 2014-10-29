#version 420

uniform mat4 MVP;

in vec3 a_position;

void main() {
    gl_Position = MVP * vec4(a_position, 1.0);
}
