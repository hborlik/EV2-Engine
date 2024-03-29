/* render.glsl - public domain
by Jonathan Dupuy

    This code has dependencies on the following GLSL sources:
    - TerrainRenderCommon.glsl
    - LongestEdgeBisection.glsl
*/

/*******************************************************************************
 * Vertex Shader
 *
 * The vertex shader is empty
 */
#ifdef VERTEX_SHADER
layout(location = 0) in vec2 i_VertexPos;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out vec3 o_WorldPos;
layout(location = 2) out vec3 o_frag_pos;

void main()
{
    const int cbtID = 0;
    uint nodeID = gl_InstanceID;
    cbt_Node node = cbt_DecodeNode(cbtID, nodeID);
    vec4 triangleVertices[3] = DecodeTriangleVertices(node);
    vec2 triangleTexCoords[3] = vec2[3](
        triangleVertices[0].xy,
        triangleVertices[1].xy,
        triangleVertices[2].xy
    );

    // compute final vertex attributes
    VertexAttribute attrib = TessellateTriangle(
        triangleTexCoords,
        i_VertexPos
    );

#if defined(PROJECTION_FISHEYE)
    vec3 screenPos = WorldToScreenSpace(attrib.position.xyz);

    screenPos.z = screenPos.z / 64000.0f; // zFar
    screenPos.xy*= 1.0f * vec2(1.0f, 1920.0f / 720.0f);
    gl_Position = vec4(screenPos, 1.0);
#else
    gl_Position = u_ModelViewProjectionMatrix * attrib.position;
#endif
    o_TexCoord  = attrib.texCoord;
    o_WorldPos  = (u_ModelMatrix * attrib.position).xyz;
    o_frag_pos = (u_ModelViewMatrix * attrib.position).xyz;
}
#endif


/*******************************************************************************
 * Fragment Shader -- Shades the terrain
 *
 */
#ifdef FRAGMENT_SHADER
layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_WorldPos;
layout(location = 2) in vec3 i_frag_pos; // fragment position in view space

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out uint gMaterialTex;
layout (location = 4) out vec4 gEmissive;

void main()
{
    NormalAlbedo na = ShadeFragmentGbuf(i_TexCoord, i_WorldPos);
    gAlbedoSpec = vec4(na.albedo, 0.0f);
    gPosition = i_frag_pos;
    gMaterialTex = u_MaterialID;
    gNormal = normalize((u_NormalMatrix * vec4(na.normal, 0)).xyz);
    gEmissive = vec4(0.0);
}
#endif
