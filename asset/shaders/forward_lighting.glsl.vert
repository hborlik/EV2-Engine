uniform vec3 lightColors[100];

in vec3 VertPos;

flat out uint instance_id;

void main() {
    gl_Position = vec4(lightColors[gl_InstanceID], 1.0);
    instance_id = gl_InstanceID;
}