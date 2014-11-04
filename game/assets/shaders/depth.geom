#version 420 core

uniform mat4 MVP[3];

//3 invocations, triangles in and triangles out
layout(invocations = 3, triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main(void) {
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = MVP[gl_InvocationID] * gl_in[gl_InvocationID].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
