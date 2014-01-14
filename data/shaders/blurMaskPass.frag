#version 420

uniform sampler2D color0;

uniform vec2 invResolution;

out vec4 finalColor;

void main(void) {
	vec2 vTexCoord = gl_FragCoord.xy * invResolution;
	vec4 valColor0 = texture(color0, vTexCoord); //xyz = color
        float threshold = 0.5;
	if(valColor0.x > threshold || valColor0.y > threshold || valColor0.z > threshold)
		finalColor = vec4(valColor0.xyz, 1.0);
	else
		finalColor = vec4(vec3(0.0),1);
}
