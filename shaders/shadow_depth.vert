#version 120

uniform mat4 u_lightSpaceMatrix;

void main() {
    gl_Position = u_lightSpaceMatrix * gl_Vertex;
}
