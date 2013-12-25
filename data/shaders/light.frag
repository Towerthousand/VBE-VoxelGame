#version 420

uniform sampler2D depth;
uniform sampler2D color0;
uniform sampler2D color1;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform mat4 invProj;
uniform float lightRadius;
uniform vec2 invResolution;

out vec4 color;

vec3 getFragPos(vec2 vTexCoord) {
	vec4 sPos = vec4(vTexCoord * 2 - 1, texture(depth, vTexCoord).x * 2 - 1, 1.0);
    sPos = invProj * sPos;
	return sPos.xyz / sPos.w;
}

vec3 decodeNormal(vec2 enc) {
    //Decode normal
	vec2 fenc = enc * 4;
	float f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	return vec3(fenc * g, 1 - f / 2);
}

void main(void) {
        vec2 vTexCoord = gl_FragCoord.xy * invResolution;
        vec3 fragmentPos = getFragPos(vTexCoord); //view space
        if(length(fragmentPos - lightPos) > lightRadius)
                discard;

    vec4 valColor0 = texture(color0, vTexCoord);
    vec4 valColor1 = texture(color1, vTexCoord);

    //material properties
    vec3 matDiffuseColor = valColor0.xyz;
    vec3 matSpecularColor = vec3(valColor1.w);

	//fragment light parameters
	vec3 lightVector = normalize(lightPos - fragmentPos); //view space
	vec3 normalVector = decodeNormal(valColor1.xy);  //view space

    //Blinn-Phong shading
    vec3 E = normalize(-fragmentPos);
	vec3 H = normalize(lightVector + E);
    float cosAlpha = clamp(dot(normalVector, H), 0.0f, 1.0f);
    float cosTheta = max(dot(normalVector, lightVector), 0.0f);
        float attenuationFactor = max(0.0, 1 - length(fragmentPos-lightPos) / lightRadius);

	color = vec4(matDiffuseColor * lightColor * cosTheta * attenuationFactor +
                                 matSpecularColor * lightColor * pow(cosAlpha, 1000) * cosTheta * attenuationFactor, 1.0f);
}
