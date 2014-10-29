#version 420

uniform sampler2D fontTex;

in vec2 vTexCoord;
in vec4 vColor;
out vec4 finalColor;

void main(void) {
    finalColor = vec4(texture(fontTex,vTexCoord)*vColor);
}
