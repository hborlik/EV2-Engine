#include "globals.glslinc"

in layout(location = 0) vec3 VertPos;
in layout(location = 1) vec3 Normal;
in layout(location = 2) vec3 VertCol;
in layout(location = 3) vec2 TexPos;

uniform mat4 M;
uniform mat3 G;

out vec3 frag_pos;      // fragment position in world space
out vec3 vert_normal;   // normal in world space
out vec3 vert_color;    // passthrough
out vec2 tex_coord;     // passthrough

void main() {
    vec4 vertV = M * vec4(VertPos, 1.0);
    frag_pos = vertV.xyz;
    gl_Position = VP * vertV;
    vert_normal = G * Normal;
    vert_color = VertCol;
    tex_coord = TexPos;
}