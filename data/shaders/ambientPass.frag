#version 420

uniform sampler2D color0;
uniform sampler2D color1;

uniform vec2 invResolution;

out vec4 finalColor;

void main(void) {
	vec2 vTexCoord = gl_FragCoord.xy * invResolution;
	vec4 valColor0 = texture(color0, vTexCoord);
	vec4 valColor1 = texture(color1, vTexCoord);

	finalColor = vec4(valColor0.xyz *(0.05 + valColor1.z), 1.0);
}
