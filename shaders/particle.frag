#version 120

uniform sampler2D u_texture;
uniform vec4 u_tint;
uniform vec3 u_lightDir;
uniform float u_depthCue;
uniform int u_antialias;

void main() {
    vec2 uv = gl_TexCoord[0].st;
    vec2 p = uv * 2.0 - 1.0;
    float radiusSq = dot(p, p);

    if (radiusSq > 1.0)
        discard;

    float edgeAlpha = 1.0;
    if (u_antialias == 1)
        edgeAlpha = 1.0 - smoothstep(0.86, 1.0, sqrt(radiusSq));

    vec3 normal = normalize(vec3(p.x, p.y, sqrt(max(1.0 - radiusSq, 0.0))));
    vec3 lightDir = normalize(u_lightDir);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float rim = pow(max(1.0 - normal.z, 0.0), 2.0);
    float depthShade = mix(0.72, 1.0, u_depthCue);
    float shade = (0.62 + diffuse * 0.42 + rim * 0.08) * depthShade;

    vec4 tex = texture2D(u_texture, uv);
    vec4 color = tex * u_tint;
    color.rgb *= clamp(shade, 0.55, 1.15);
    color.rgb += vec3(0.035, 0.04, 0.055) * (1.0 - u_depthCue);
    color.a *= edgeAlpha;

    gl_FragColor = color;
}
