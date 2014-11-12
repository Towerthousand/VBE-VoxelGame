#version 440 core

uniform mat4 VP[4];
uniform ivec3 transforms[400];

in uint g_draw_index[];

//3 invocations, triangles in and triangles out
layout(invocations = 4, triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main(void) {
    gl_Position = VP[gl_InvocationID] * (gl_in[0].gl_Position+vec4(transforms[g_draw_index[0]], 0));
    gl_Layer = gl_InvocationID;
    EmitVertex();
    gl_Position = VP[gl_InvocationID] * (gl_in[1].gl_Position+vec4(transforms[g_draw_index[1]], 0));
    gl_Layer = gl_InvocationID;
    EmitVertex();
    gl_Position = VP[gl_InvocationID] * (gl_in[2].gl_Position+vec4(transforms[g_draw_index[2]], 0));
    gl_Layer = gl_InvocationID;
    EmitVertex();
    EndPrimitive();
}
