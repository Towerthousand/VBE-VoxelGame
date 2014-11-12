#version 440 core

uniform mat4 VP[4];
uniform ivec3 transforms[400];

in uint g_draw_index[];

//3 invocations, triangles in and triangles out
layout(invocations = 4, triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main(void) {
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = VP[gl_InvocationID] * (gl_in[i].gl_Position+vec4(transforms[g_draw_index[i]], 0));
        // Assign gl_InvocationID to gl_Layer to direct rendering
        // to the appropriate layer
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
