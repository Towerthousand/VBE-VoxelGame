#version 420

uniform sampler2D color0;
uniform sampler2D color1;
uniform sampler2DArrayShadow sunDepth;
uniform sampler2D depth;
uniform mat4 depthMVP[4];
uniform float depthPlanes[4];
uniform mat4 invCamProj;
uniform mat4 invCamView;
uniform mat4 camMV;
uniform vec2 invResolution;
uniform vec3 lightDir;

out vec4 finalColor;

vec2 poissonDisk[16] = {
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
};

vec4 skyColor = vec4(50.0f/255.0f,90.0f/255.0f,255.0f/255.0f,1);

vec3 getFragPos(vec2 texCoord) {
    vec4 sPos = vec4(texCoord * 2 - 1, texture(depth, texCoord).x * 2 - 1, 1.0);
    sPos = invCamProj * sPos;
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
    vec4 valColor0 = texture(color0, vTexCoord);
    vec4 valColor1 = texture(color1, vTexCoord);

    vec3 fragmentViewPos = getFragPos(vTexCoord); //view space
    int shadowIndex = 0;
    for(int i = 0; i < 4; ++i) {
        shadowIndex = i;
        if(abs(fragmentViewPos.z) < depthPlanes[i]) break;
    }
    vec3 fragmentWorldPos = vec4(invCamView*vec4(fragmentViewPos,1.0)).xyz; //world space
    vec4 shadowCoord = vec4(depthMVP[shadowIndex]*vec4(fragmentWorldPos,1.0)); //texture space (for shadow tex)

    vec3 normalVector = normalize(decodeNormal(valColor1.xy));  //view space
    vec3 lightVector = normalize(vec4(camMV*vec4(lightDir,0.0)).xyz);

    float cosTheta = max(-dot(lightVector, normalVector), 0.0f);

    // Compute visibility. Sample the shadow map 16 times
    float visibility = 1.0;
    float bias = 0.0025f;
    float shadowZ = (shadowCoord.z-bias)/shadowCoord.w;
    float sampleNum = 16.0f;

    for (int i=0; i < sampleNum; i++)
        visibility -= (1.0f/sampleNum)*(texture(sunDepth,vec4(shadowCoord.xy + poissonDisk[i]/1000.0, shadowIndex, shadowZ)));

    // Abs distance to player:
    vec4 camPos = invCamView * vec4(vec3(0.0), 1.0);

    //Compute fog percentage
    float fog = clamp(length(fragmentWorldPos - camPos.xyz)/(16.0*16.0), 0.0, 1.0);
    fog = pow(fog, 9);
    finalColor = vec4(vec3(valColor0.xyz*(0.05 + valColor1.z + visibility*cosTheta*0.6))*(1-fog) + skyColor.xyz*fog, 1.0);
}
