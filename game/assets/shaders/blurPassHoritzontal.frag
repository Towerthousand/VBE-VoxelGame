#version 420

uniform sampler2D RTScene; // the texture with the scene you want to blur
uniform vec2 invResolution;

out vec4 color;

const float blurSize = invResolution.x; // I've chosen this size because this will result in that every step will be one pixel wide if the RTScene texture is of size 512x512

void main(void) {
   vec2 texCoord = gl_FragCoord.xy * invResolution;
   vec4 sum = vec4(0.0);
   // blur in y (vertical)
   // take eleven samples, with the distance blurSize between them 0.
   sum += texture2D(RTScene, vec2(texCoord.x - 5.0*blurSize, texCoord.y)) * 0.02655802906;
   sum += texture2D(RTScene, vec2(texCoord.x - 4.0*blurSize, texCoord.y)) * 0.04505334779;
   sum += texture2D(RTScene, vec2(texCoord.x - 3.0*blurSize, texCoord.y)) * 0.06281566324;
   sum += texture2D(RTScene, vec2(texCoord.x - 2.0*blurSize, texCoord.y)) * 0.09528487833;
   sum += texture2D(RTScene, vec2(texCoord.x - blurSize, texCoord.y)) * 0.12234820561;
   sum += texture2D(RTScene, vec2(texCoord.x, texCoord.y)) * 0.13298076013;
   sum += texture2D(RTScene, vec2(texCoord.x + blurSize, texCoord.y)) * 0.12234820561;
   sum += texture2D(RTScene, vec2(texCoord.x + 2.0*blurSize, texCoord.y)) * 0.09528487833;
   sum += texture2D(RTScene, vec2(texCoord.x + 3.0*blurSize, texCoord.y)) * 0.06281566324;
   sum += texture2D(RTScene, vec2(texCoord.x + 4.0*blurSize, texCoord.y)) * 0.04505334779;
   sum += texture2D(RTScene, vec2(texCoord.x + 5.0*blurSize, texCoord.y)) * 0.02655802906;

   color = vec4(sum.xyz, 1.0);
}
