#version 420

uniform sampler2D depth;
uniform sampler2D color0;
uniform sampler2D color1;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform mat4 invProj;
uniform mat4 invView;
uniform float lightRadius;
uniform vec2 invResolution;

uniform sampler3D tex;

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

        //fragment light parameters
        vec3 lightVector = lightPos - fragmentPos; //view space
        float lightDist = length(lightVector);
        lightVector /= lightDist;

        if(lightDist > lightRadius)
                discard;

        vec4 valColor0 = texture(color0, vTexCoord);
        vec4 valColor1 = texture(color1, vTexCoord);

        //material properties
        vec3 matDiffuseColor = valColor0.xyz;
        vec3 matSpecularColor = vec3(valColor1.w);
        vec3 normalVector = decodeNormal(valColor1.xy);  //view space

        vec3 posRelativeToLight = (invView * vec4(fragmentPos+normalVector*0.1 - lightPos, 0.0f)).xyz;
        float blockLight = texture(tex, (posRelativeToLight+0.5)/(2*lightRadius) + 0.5f).r;
        if(blockLight < 0.01)
           discard;

		//Blinn-Phong shading
        vec3 E = normalize(-fragmentPos);
        vec3 H = normalize(lightVector + E);
        float cosAlpha = max(dot(normalVector, H), 0.0f);
        float cosTheta = max(dot(normalVector, lightVector), 0.0f);
        float attenuationFactor = 1 - lightDist / lightRadius;

        gl_FragDepth = texture(depth, vTexCoord).x;

        color = vec4(lightColor * 10.0f * cosTheta * attenuationFactor * blockLight * (matDiffuseColor + matSpecularColor * pow(cosAlpha, 1000) * blockLight), 1.0f);
}
