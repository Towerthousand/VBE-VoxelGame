#version 420

uniform mat4 MVP;

in vec2 a_position;
in vec2 a_texCoord;
in vec4 a_color;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vTexCoord = a_texCoord;
    vColor = a_color;
    gl_Position = MVP * vec4(vec3(a_position, 0.0f), 1.0);
}
