

out layout(location = 0) vec4 frag_color;
in vec2 tex_coord;

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBuffer;

uniform float exposure = 0.5;
uniform float gamma = 2.2;
uniform float bloom_falloff = 1.0;

void main() {
    const vec3 hdrColor = texture(hdrBuffer, tex_coord).rgb;

    if (any(lessThan(hdrColor.rgb, vec3(0)))) {
        frag_color = vec4(1, 0, 0, 1);
        return;
    }

    const vec3 bColor = texture(bloomBuffer, tex_coord).rgb;
    vec3 color = hdrColor + pow(bColor * pow(bColor - hdrColor, vec3(2)), vec3(bloom_falloff));

    // exposure tone mapping
    color = vec3(1.0) - exp(-color * exposure);
    // Reinhard
    // color = color / (color + vec3(1.0));
    // gamma correction 
    color = pow(color, vec3(1.0 / gamma));

    frag_color = vec4(color, 1);

    if (any(isnan(color.rgb)))
        frag_color = vec4(1, 0, 0, 1);
}