#version 120

const int MAX_LIGHTS = 8;

uniform sampler2D u_texture;
uniform sampler2D u_shadowMap;
uniform vec4 u_tint;
uniform vec3 u_viewPosition;
uniform vec3 u_viewRight;
uniform vec3 u_viewUp;
uniform vec3 u_viewForward;
uniform vec3 u_particleCenter;
uniform float u_particleRadius;
uniform float u_depthCue;
uniform int u_lightCount;
uniform vec3 u_lightPositions[MAX_LIGHTS];
uniform float u_lightRanges[MAX_LIGHTS];
uniform float u_lightIntensities[MAX_LIGHTS];
uniform int u_antialias;
uniform int u_shadowEnabled;
uniform float u_shadowMapSize;
uniform mat4 u_lightSpaceMatrix;

float shadowFactor(vec3 surfacePos, vec3 normal, vec3 lightDir) {
    if (u_shadowEnabled == 0)
        return 1.0;

    vec4 lightSpace = u_lightSpaceMatrix * vec4(surfacePos, 1.0);
    vec3 proj = lightSpace.xyz / lightSpace.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
        return 1.0;

    float bias = max(0.0015 * (1.0 - dot(normal, lightDir)), 0.00045);
    float texel = 1.0 / max(u_shadowMapSize, 1.0);
    float lit = 0.0;

    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            float closest = texture2D(u_shadowMap, proj.xy + vec2(float(x), float(y)) * texel).r;
            lit += (proj.z - bias <= closest) ? 1.0 : 0.0;
        }
    }

    return mix(0.38, 1.0, lit / 9.0);
}

void main() {
    vec2 uv = gl_TexCoord[0].st;
    vec2 p = uv * 2.0 - 1.0;
    float radiusSq = dot(p, p);

    if (radiusSq > 1.0)
        discard;

    float edgeAlpha = 1.0;
    if (u_antialias == 1)
        edgeAlpha = 1.0 - smoothstep(0.86, 1.0, sqrt(radiusSq));

    float z = sqrt(max(1.0 - radiusSq, 0.0));
    vec3 normal = normalize(u_viewRight * p.x + u_viewUp * p.y - u_viewForward * z);
    vec3 surfacePos = u_particleCenter + normal * u_particleRadius;
    vec3 viewDir = normalize(u_viewPosition - surfacePos);
    vec3 lighting = vec3(0.18, 0.19, 0.22);

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= u_lightCount)
            break;

        vec3 lightVector = u_lightPositions[i] - surfacePos;
        float distanceToLight = length(lightVector);

        if (distanceToLight <= 0.0001)
            continue;

        vec3 lightDir = lightVector / distanceToLight;
        float attenuation = clamp(1.0 - distanceToLight / max(u_lightRanges[i], 1.0), 0.0, 1.0);
        attenuation *= attenuation;

        float diffuse = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        float shadow = (i == 0) ? shadowFactor(surfacePos, normal, lightDir) : 1.0;

        vec3 lightColor = vec3(1.0, 0.88, 0.58);
        lighting += lightColor * u_lightIntensities[i] * attenuation * shadow * (diffuse * 1.25 + specular * 0.55);
    }

    float rim = pow(max(1.0 - dot(normal, viewDir), 0.0), 2.0);
    float depthShade = mix(0.72, 1.0, u_depthCue);
    lighting += vec3(0.08, 0.1, 0.13) * rim;
    lighting *= depthShade;

    vec4 tex = texture2D(u_texture, uv);
    vec4 color = tex * u_tint;
    color.rgb *= clamp(lighting, vec3(0.12), vec3(1.65));
    color.rgb += vec3(0.035, 0.04, 0.055) * (1.0 - u_depthCue);
    color.a *= edgeAlpha;

    gl_FragColor = color;
}
