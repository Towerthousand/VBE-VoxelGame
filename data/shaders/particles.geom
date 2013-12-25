#version 420 core //4.2

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform float texSize; // 1.0 / num de texturas

in vec2 geom_vel[];
in vec4 geom_color[];
in float geom_size[];
in int geom_texIndex[];

out vec2 v_texCoord;
out vec4 v_color;
out float v_z;
out float v_size;

const vec2[4] texCoords = {
    vec2( 1, 1),
    vec2( 0, 1),
    vec2( 1, 0),
    vec2( 0, 0)
};

const vec2[4] displacements = {
    vec2( 1, 1),
    vec2(-1, 1),
    vec2( 1, -1),
    vec2(-1, -1)
};

void main() {
    vec3 front = normalize(gl_in[0].gl_Position.xyz);
    vec3 vel = vec3(geom_vel[0], 0.0f);

    vec3 up = cross(front, vel);

    mat3 transform = mat3(1.0);

    if(length(up) < 0.0001) {
        up = vec3(0.0, 1.0, 0.0);
        vec3 right = normalize(cross(front, up));
        up = cross(front, right);
        transform = mat3(right, up, front);
    }
    else {
        up = normalize(up);
        vec3 right = normalize(cross(front, up));
        float len = length(vel);
        transform = mat3(right*(1+len*0.2), up, front);
    }
    for(int i = 0; i < 4; i++) {
        // copy attributes
        vec3 disp = transform*vec3(displacements[i], 0.0);
        vec4 viewPos = (gl_in[0].gl_Position + vec4(disp * geom_size[0], 0.0));
        v_z = viewPos.z;
        gl_Position = projectionMatrix * viewPos;
        v_texCoord.x = texSize*(float(geom_texIndex[0]) + float(texCoords[i].x));
        v_texCoord.y = texCoords[i].y;
        v_color = geom_color[0];
        v_size = geom_size[0];
        // done with the vertex
        EmitVertex();
    }
    EndPrimitive();
}
