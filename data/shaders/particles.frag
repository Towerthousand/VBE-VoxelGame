#version 420 core

uniform sampler2D textureSheet;
uniform sampler2D depth;
uniform vec2 invResolution;
uniform mat4 invProj;

in vec2 v_texCoord;
in vec4 v_color;
in float v_z;
in float v_size;

out vec4 outColor;

void main() {
    vec2 screenCoord = gl_FragCoord.xy*invResolution;
    vec4 col = texture(textureSheet, v_texCoord);

    float particleZ = v_z;
    float particleZMax = v_z+col.a*v_size;
    float particleZMin = v_z-col.a*v_size;

    vec4 sPos = vec4(screenCoord*2-1, texture(depth, screenCoord).x*2-1, 1.0);
    sPos = invProj * sPos;
    vec3 fragViewPos = sPos.xyz/sPos.w;

    float modelZ = fragViewPos.z;

    float visibility = (particleZMax - modelZ) / (particleZMax - particleZMin);

    if(visibility <= 0)
        discard;

    visibility = min(visibility, 1.0);
    col.a *= visibility;

    outColor = v_color*col;
}
