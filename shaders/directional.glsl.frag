#include "globals.glslinc"
#include "disney.glslinc"

out vec4 frag_color;

in vec2 tex_coord;

uniform mat4 LS;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform usampler2D gMaterialTex;
uniform sampler2D gAO;
uniform sampler2D shadowDepth;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightAmbient;


/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float PCF(in vec3 LSfPos, in vec3 Normal) {
    // outside shadow area
    if (any(greaterThan(abs(2 * LSfPos - 1.0f), vec3(1.0f))))
        return 0.f;
    // pcss
    vec3 X = vec3(1, 0, 0);
    vec3 Y = normalize(cross(Normal, X));
    X = normalize(cross(Y, Normal));

    mat3 TBN = mat3(X, Y, Normal);

    vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
    float percentShadow = 0.0f;
    // 5x5 sampling
    for (int i = -2; i <= 2; i++)
        for (int j = -2; j <= 2; j++) {
            const vec3 spos = TBN * vec3(vec2(i, j) * texelScale, 0);
            const float lightDepth = texture(shadowDepth, LSfPos.xy + spos.xy).r;
            if (LSfPos.z - abs(spos.z * 1.5f) > lightDepth)
                percentShadow += 1.0f;
        }

    return percentShadow / 25.0f;
}

// BRDF shader interface

void main() {
    vec3 FragPosView = texture(gPosition, tex_coord).rgb;
    if (FragPosView == vec3(0, 0, 0)) // no rendered geometry
        discard;

    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 vNormal = normalize((View * vec4(Normal, 0)).xyz);
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;
    uint MaterialId = texture(gMaterialTex, tex_coord).r;
    float AO = texture(gAO, tex_coord).r;

    // constant tangent spaces
    vec3 X = vec3(1, 0, 0);
    vec3 Y = normalize(cross(vNormal, X));
    X = normalize(cross(Y, vNormal));

    vec3 lightDirV = vec3(View * vec4(lightDir, .0f));

    vec3 viewDir = normalize(-FragPosView);

    mat4 inv_pv = LS * VInv;
    vec4 LSfPos = (inv_pv * vec4(FragPosView, 1.0));
    float Shade = PCF(LSfPos.xyz, (LS * vec4(Normal, 0.0)).xyz);

    vec3 diffuse = pow(materials[MaterialId].diffuse, vec3(2.2)); // diffuse is srgb.
    // vec3 color = /*AO*/0 * lightAmbient * (Albedo + diffuse) + (1.0 - Shade) * lightColor * BRDF(lightDirV, viewDir, vNormal, X, Y, Albedo, materials[MaterialId]);
    vec3 color = (1.0 - Shade) * lightColor;// * BRDF(lightDirV, viewDir, vNormal, X, Y, Albedo, materials[MaterialId]);
    
    frag_color = vec4(color, 1.0);
    // frag_color = vec4(AO, AO, AO, 1.0);
    // frag_color = vec4(texture(shadowDepth, (inv_pv * vec4(FragPos, 1.0)).xy).r);
    // frag_color = vec4(Normal, 0);
}