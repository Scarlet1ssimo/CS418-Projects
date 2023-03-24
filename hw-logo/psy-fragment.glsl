#version 300 es
precision highp float;

in vec4 vColor;

uniform float seconds;

out vec4 fragColor;

void main() {
    float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    float c1x = sin((sin(seconds * 1.1 + 5.) * 1.6 + 1.5) * x / 10.);
    float c2x = sin((sin(seconds * 1.2 + 4.) * 1.9 + 1.3) * x / 11.);
    float c1y = sin((sin(seconds * 1.3 + 3.) * 1.7 + 1.6) * y / 13.);
    float c2y = sin((sin(seconds * 1.4 + 2.) * 1.4 + 1.2) * y / 14.);
    // c2 = sin(seconds * y + 0.2);
    float wari = (c1x + c2y + c2x + c1y) / 4.;//[-1,1]
    float r1 = wari < 0. ? 0. : wari;
    float r2 = wari < 0. ? -wari : 0.;
    fragColor = vec4(r1, r2, r1, 1);
}