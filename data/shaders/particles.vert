#version 420 core

uniform mat4 modelViewMatrix;

in vec3 a_position;
in vec3 a_vel;
in vec4 a_color;
in float a_size;
in int a_texIndex;

out vec2 geom_vel;
out vec4 geom_color;
out float geom_size;
out int geom_texIndex;

void main () {
    gl_Position = modelViewMatrix * vec4(a_position, 1.0);
    geom_color = a_color;
    geom_size = a_size;
    geom_texIndex = a_texIndex;
    geom_vel = vec3(mat3(modelViewMatrix)*a_vel).xy;
}
