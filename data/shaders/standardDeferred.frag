#version 420

uniform sampler2D diffuseTex;
uniform float ambient;
uniform float specular;

in vec2 v_texCoord;
in vec3 v_normal;

layout(location = 0) out vec3 color0;
layout(location = 1) out vec4 color1;

void main(void) {
        color0 = texture(diffuseTex, v_texCoord).xyz;

        vec3 normal = normalize(v_normal);
        float p = sqrt(normal.z*8+8);
        vec2 encodedNormal = normal.xy/p;
        color1 = vec4(encodedNormal, ambient, specular);
}
