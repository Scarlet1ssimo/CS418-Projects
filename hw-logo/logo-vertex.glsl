#version 300 es

in vec4 position;
in vec4 color;

uniform mat4 model;
uniform float seconds;

out vec4 vColor;

void main() {
    vColor = color;
    if(gl_InstanceID == 0) {//logo I
        gl_Position = model * vec4(position.xy, position.zw);
    }
}
