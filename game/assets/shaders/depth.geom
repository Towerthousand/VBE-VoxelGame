#version 420 core

uniform mat4 MVP[3];

//3 invocations, triangles in and triangles out
layout(invocations = 3, triangles) in;
layout(triangle_strips, max_vertices = 3) in;

void main(void) {
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = MVP[gl_InvocationID] * gl_in[gl_InvocationID].gl_Position;
        // Assign gl_InvocationID to gl_Layer to direct rendering
        // to the appropriate layer
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
