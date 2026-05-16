#version 120

uniform sampler2D u_texture;
uniform vec4 u_tint;

void main() {
    vec4 tex = texture2D(u_texture, gl_TexCoord[0].st);
    gl_FragColor = tex * u_tint;
}
