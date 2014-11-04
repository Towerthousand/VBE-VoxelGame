#version 440 core

uniform mat4 MVP[4];

//3 invocations, triangles in and triangles out
layout(invocations = 4, triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main(void) {
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = MVP[gl_InvocationID] * gl_in[i].gl_Position;
        // Assign gl_InvocationID to gl_Layer to direct rendering
        // to the appropriate layer
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
