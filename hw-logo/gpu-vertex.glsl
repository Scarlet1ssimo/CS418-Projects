#version 300 es

in vec4 position;
in vec4 color;

uniform mat4 model;
uniform float seconds;

out vec4 vColor;
void main() {
    vColor = color;
    gl_Position = position;
    if(gl_InstanceID == 0) {//logo I
        float ss1 = seconds * 0.2;
        float scale = abs(seconds - floor(seconds) - 0.5) * 0.02 + 0.02;
        float dx = max(min(abs(ss1 - floor(ss1) - 0.5), .375), .125) * 4.0 - 1.;
        float dy = max(min(abs(ss1 + .25 - floor(ss1 + .25) - 0.5), .375), .125) * 4.0 - 1.;
        float x = (position.x - 4.5) * scale;
        float y = (position.y - 7.) * scale;
        gl_Position = vec4(sin(seconds) * x + cos(seconds) * y + dx, sin(seconds) * y - cos(seconds) * x + dy, position.zw);
    }
}
