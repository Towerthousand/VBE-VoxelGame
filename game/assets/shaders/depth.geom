#version 440 core

uniform mat4 VP[4];
uniform ivec3 transforms[400];

in uint g_draw_index[];

layout(invocations = 4, triangles) in;
layout(triangle_strip, max_vertices = 3) out;

vec4 positions[4];
vec4 positionsTransformed[4];

// return 1 if v inside the box from (-1,-1) to (1,1), return 0 otherwise
float insideBox(vec2 v) {
    vec2 s = step(vec2(-1,-1), v) - step(vec2(1,1), v);
    return s.x * s.y;
}

void main(void) {
    //manual clipping reduces further throughput.
    positions[0] = VP[gl_InvocationID] * (gl_in[0].gl_Position+vec4(transforms[g_draw_index[0]], 0));
    positions[1] = VP[gl_InvocationID] * (gl_in[1].gl_Position+vec4(transforms[g_draw_index[1]], 0));
    positions[2] = VP[gl_InvocationID] * (gl_in[2].gl_Position+vec4(transforms[g_draw_index[2]], 0));
    positionsTransformed[0] = positions[0]/positions[0].w;
    positionsTransformed[1] = positions[0]/positions[1].w;
    positionsTransformed[2] = positions[0]/positions[2].w;

    if(!(insideBox(positionsTransformed[0].xy) > 0.0 || insideBox(positionsTransformed[1].xy) > 0.0 || insideBox(positionsTransformed[2].xy) > 0.0)) return;

    //actually emit geometry
    gl_Layer = gl_InvocationID;
    gl_Position = positions[0];
    EmitVertex();
    gl_Position = positions[1];
    EmitVertex();
    gl_Position = positions[2];
    EmitVertex();
    EndPrimitive();
}
