#version 420

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform vec2 invResolution;

out vec4 finalColor;

void main(void) {
    vec2 vTexCoord = gl_FragCoord.xy*invResolution;
    finalColor = vec4(texture(tex1,vTexCoord).xyz + texture(tex2,vTexCoord).xyz*1,1.0);
}
