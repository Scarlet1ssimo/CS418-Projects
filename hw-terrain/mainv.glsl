#version 300 es
in vec4 position;
in vec3 normal;
out vec3 outnormal;
out vec3 fnormal;
uniform mat4 p;
uniform mat4 mv;
uniform vec2 maxmin;
out float height;
void main() {
    height = (position.z - maxmin.y) / (maxmin.x - maxmin.y) * 2. - 1.;
    gl_Position = p * mv * position;
    outnormal = normal;
    fnormal = mat3(mv) * normal;
}